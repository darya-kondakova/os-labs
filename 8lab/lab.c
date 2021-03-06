#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <float.h>

#define CHECK_STEP 1000000
#define MAX_THREADS_AMOUNT 800
#define SUCCESS 0
#define ERRNO_SET 0
#define ERROR_EXIT 1

int threads_amount;
int exit_flag = 0;
int threads_exit_flag = 0;
pthread_barrier_t barrier;

double MAX_MIL_ITER = (1.0 - 3.0 * DBL_EPSILON) / (4.0 * DBL_EPSILON * CHECK_STEP) - 1;

void error_exit(const char *const msg, int error) {
    if (error != ERRNO_SET) {
        fprintf(stderr, "%s: %s", msg, strerror(error));
    } else {
        if (errno == 0) {
            fprintf(stderr, "%s\n", msg);
        } else {
            perror(msg);
        }
    }
    exit(ERROR_EXIT);
}

typedef struct thread_struct {
    int thread_num;
    double res;
} thread_t;

void my_pthread_barrier_wait() {
    int error = pthread_barrier_wait(&barrier);
    if (error != SUCCESS && error != PTHREAD_BARRIER_SERIAL_THREAD) {
        error_exit("pthread_barrier_wait() failed", error);
    }
}

void *pi_calc(void* arg) {
    double pi_part = 0.0;
    for (double j = 0; j < MAX_MIL_ITER; j++)  {
        if (exit_flag && !threads_exit_flag) {
            threads_exit_flag = 1;
        }
        my_pthread_barrier_wait();
        if (threads_exit_flag) {
            ((thread_t *)arg)->res = pi_part;
            pthread_exit(NULL);
        }
        my_pthread_barrier_wait();
        for (double i = ((thread_t *)arg)->thread_num; i < CHECK_STEP; i += threads_amount) {
            pi_part += 1 / (4.0 * (j * CHECK_STEP + i) + 1.0);
            pi_part -= 1 / (4.0 * (j * CHECK_STEP + i) + 3.0);
        }
    }
}

void exit_sig() {
    exit_flag = 1;
}

void check_args(int argc, char **argv) {
    if (argc < 2) {
        error_exit("Usage: ./a.out threads_amount", ERRNO_SET);
    }
    threads_amount = atoi(argv[1]);
    if (threads_amount < 1 || MAX_THREADS_AMOUNT < threads_amount) {
        error_exit("invalid number of threads", ERRNO_SET);
    }
}

int main(int argc, char **argv) {
    check_args(argc, argv);

    sigset(SIGINT, exit_sig);

    int error = pthread_barrier_init(&barrier, NULL, threads_amount);
    if (error != SUCCESS) {
        error_exit("pthread_barrier_init() failed", error);
    }

    pthread_t *child = (pthread_t *)malloc(threads_amount * sizeof(pthread_t));
    if (child == NULL) {
        error_exit("failed to allocate memory for threads", ERRNO_SET);
    }

    thread_t *thread_struct = (thread_t *)malloc(threads_amount * sizeof(thread_t));
    if (thread_struct == NULL) {
        error_exit("failed to allocate memory for thread_struct", ERRNO_SET);
    }

    for (int i = 0; i < threads_amount; i++) {
        thread_struct[i].thread_num = i;
        error = pthread_create(&child[i], NULL, pi_calc, &thread_struct[i]);
        if (error != SUCCESS) {
            error_exit("pthread_create() failed", error);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < threads_amount; i++) {
        error = pthread_join(child[i], NULL);
        if (error != SUCCESS) {
            error_exit("pthread_join() failed", error);
        }
        pi += thread_struct[i].res;
    }

    printf("\nπ = %.15lf\n", pi * 4);

    error = pthread_barrier_destroy(&barrier);
    if (error != SUCCESS) {
        error_exit("pthread_barrier_destroy() failed", error);
    }
    free(child);
    free(thread_struct);

    pthread_exit(NULL);
}
