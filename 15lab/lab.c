#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

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

void my_sem_init(sem_t **sem1, sem_t **sem2) {
    *sem1 = sem_open("/sem1", O_CREAT, S_IRWXU, 1);
    *sem2 = sem_open("/sem2", O_CREAT, S_IRWXU, 0);
    if (*sem1 == SEM_FAILED || *sem2 == SEM_FAILED) {
        error_exit("sem_open() failed", ERRNO_SET);
    }
}

void my_sem_close(sem_t **sem1, sem_t **sem2) {
    int error = sem_close(*sem1);
    if (error == FAIL) {
        error_exit("sem_close() failed", ERRNO_SET);
    }
    error = sem_close(*sem2);
    if (error == FAIL) {
        error_exit("sem_close() failed", ERRNO_SET);
    }
    sem_unlink("/sem1");
    sem_unlink("/sem2");
}

void child_body() {
    sem_t *sem1, *sem2;
    my_sem_init(&sem1, &sem2);
    for (int i = 0; i < 10; i++) {
        int error = sem_wait(sem2);
        if (error == FAIL) {
            error_exit("sem_wait() failed: unable to lock 2nd semaphore", ERRNO_SET);
        }
        sleep(1);
        printf("child\n");
        error = sem_post(sem1);
        if (error == FAIL) {
            error_exit("sem_post() failed: unable to unlock 1st semaphore", ERRNO_SET);
        }
    }
    my_sem_close(&sem1, &sem2);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        error_exit("Usage: ./a.out thread_number", ERRNO_SET);
    }
    int thread_number = atoi(argv[1]);
    if (thread_number != 0) {
        child_body();
    } else {
        sem_t *sem1, *sem2;
        my_sem_init(&sem1, &sem2);
        for (int i = 0; i < 10; i++) {
            int error = sem_wait(sem1);
            if (error == FAIL) {
                error_exit("sem_wait() failed: unable to lock 1st semaphore", ERRNO_SET);
            }
            sleep(1);
            printf("parent\n");
            error = sem_post(sem2);
            if (error == FAIL) {
                error_exit("sem_post() failed: unable to unlock 2nd semaphore", ERRNO_SET);
            }
        }
        my_sem_close(&sem1, &sem2);
    }
}
