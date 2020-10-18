#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define SUCCESS 0
#define FAIL -1
#define ERRNO_SET 0
#define ERROR_EXIT 1

#define MAX_A 100
#define MAX_B 80
#define MAX_C 60
#define MAX_AB 60
#define MAX_WIDGET 3

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

sem_t sem[5];   //sem[0] for A
                //sem[1] for B
                //sem[2] for C
                //sem[3] for AB

pthread_t   creater[4];

void* create_A(void* arg) {
    for (;;) {
        pthread_testcancel();
        int A;
        sem_getvalue(&sem[0], &A);
        if (A != MAX_A) {
            sleep(1);
            int error = sem_post(&sem[0]);
            if (error == FAIL) {
                error_exit("sem_post() failed: unable to create A", ERRNO_SET);
            }
            printf("new A\n");
        }
    }
}

void* create_B(void* arg) {
    for (;;) {
        pthread_testcancel();
        int B;
        sem_getvalue(&sem[1], &B);
        if (B != MAX_B) {
            sleep(2);
            int error = sem_post(&sem[1]);
            if (error == FAIL) {
                error_exit("sem_post() failed: unable to create B", ERRNO_SET);
            }
            printf("new B\n");
        }
    }
}

void* create_C(void* arg) {
    for (;;) {
        pthread_testcancel();
        int C;
        sem_getvalue(&sem[2], &C);
        if (C != MAX_C) {
            sleep(3);
            int error = sem_post(&sem[2]);
            if (error == FAIL) {
                error_exit("sem_post() failed: unable to create C", ERRNO_SET);
            }
            printf("new C\n");
        }
    }
}

void* create_AB(void* arg) {
    for (;;) {
        pthread_testcancel();
        int AB;
        sem_getvalue(&sem[3], &AB);
        if (AB != MAX_AB) {
            int error = sem_wait(&sem[0]);
            if (error == FAIL) {
                error_exit("sem_wait() failed: unable to use A", ERRNO_SET);
            }
            error = sem_wait(&sem[1]);
            if (error == FAIL) {
                error_exit("sem_wait() failed: unable to use B", ERRNO_SET);
            }
            error = sem_post(&sem[3]);
            if (error == FAIL) {
                error_exit("sem_post() failed: unable to create AB", ERRNO_SET);
            }
            printf("new AB\n");
        }
    }
}

void* create_widget() {
    for (;;) {
        pthread_testcancel();
        int widget;
        sem_getvalue(&sem[4], &widget);
        if (widget != MAX_WIDGET) {
            int error = sem_wait(&sem[3]);
            if (error == FAIL) {
                error_exit("sem_wait() failed: unable to use AB", ERRNO_SET);
            }
            error = sem_wait(&sem[2]);
            if (error == FAIL) {
                error_exit("sem_wait() failed: unable to use C", ERRNO_SET);
            }
            error = sem_post(&sem[4]);
            if (error == FAIL) {
                error_exit("sem_post() failed: unable to create widget", ERRNO_SET);
            }
            printf("new widget\n");
        }
    }
}

void exit_sig() {
    int error;
    for (int i = 0; i < 4; i++) {
        error = pthread_cancel(creater[i]);
        if (error != SUCCESS) {
            error_exit("pthread_cancel() failed", error);
        }
    }
    for (int i = 0; i < 5; i++) {
        error = sem_destroy(&sem[i]);
        if (error == FAIL) {
            error_exit("sem_destroy() failed", ERRNO_SET);
        }
    }
    pthread_exit(NULL);
}

int main() {
    sigset(SIGINT, exit_sig);

    int error;
    for (int i = 0; i < 5; i++) {
        error = sem_init(&sem[i], 0, 0);
        if (error == FAIL) {
            error_exit("sem_init() failed: unable to initialize semaphore", ERRNO_SET);
        }
    }

    error = pthread_create(&creater[0], NULL, create_A, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }
    error = pthread_create(&creater[1], NULL, create_B, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }
    error = pthread_create(&creater[2], NULL, create_C, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }
    error = pthread_create(&creater[3], NULL, create_AB, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }

    create_widget();
}
