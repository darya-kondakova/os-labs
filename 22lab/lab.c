#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#define SUCCESS 0
#define FAIL -1
#define ERRNO_SET 0
#define ERROR_EXIT 1

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

sem_t sem[4];   //sem[0] for A
                //sem[1] for B
                //sem[2] for C
                //sem[3] for AB

void* create_A(void* arg) {
    for (;;) {
        sleep(1);
        int error = sem_post(&sem[0]);
        if (error == FAIL) {
            error_exit("sem_post() failed: unable to create A", ERRNO_SET);
        }
        printf("new A\n");
    }
}

void* create_B(void* arg) {
    for (;;) {
        sleep(2);
        int error = sem_post(&sem[1]);
        if (error == FAIL) {
            error_exit("sem_post() failed: unable to create B", ERRNO_SET);
        }
        printf("new B\n");
    }
}

void* create_C(void* arg) {
    for (;;) {
        sleep(3);
        int error = sem_post(&sem[2]);
        if (error == FAIL) {
            error_exit("sem_post() failed: unable to create C", ERRNO_SET);
        }
        printf("new C\n");
    }
}

void* create_AB(void* arg) {
    for (;;) {
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

void* create_widget() {
    for (;;) {
        int error = sem_wait(&sem[3]);
        if (error == FAIL) {
            error_exit("sem_wait() failed: unable to use AB", ERRNO_SET);
        }
        error = sem_wait(&sem[2]);
        if (error == FAIL) {
            error_exit("sem_wait() failed: unable to use C", ERRNO_SET);
        }
        printf("new widget\n");
    }
}

int main() {
    pthread_t   creating_A,
                creating_B,
                creating_C,
                creating_AB;

    int error;
    for (int i = 0; i < 4; i++) {
        error = sem_init(&sem[i], 0, 0);
        if (error == FAIL) {
            error_exit("sem_init() failed: unable to initialize semaphore", ERRNO_SET);
        }
    }

    error = pthread_create(&creating_A, NULL, create_A, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }
    error = pthread_create(&creating_B, NULL, create_B, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }
    error = pthread_create(&creating_C, NULL, create_C, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }
    error = pthread_create(&creating_AB, NULL, create_AB, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }

    create_widget();
}
