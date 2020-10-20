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
    sem_close(*sem1);
    if (error == FAIL) {
        error_exit("sem_close() failed", ERRNO_SET);
    }
    sem_close(*sem2);
    if (error == FAIL) {
        error_exit("sem_close() failed", ERRNO_SET);
    }
}

void child_body() {
    sem_t *sem1, *sem2;
    my_sem_init(&sem1, &sem2);
    for (int i = 0; i < 10; i++) {
        int error = sem_wait(sem2);
        if (error == FAIL) {
            error_exit("sem_wait() failed: unable to lock 2nd semaphore", ERRNO_SET);
        }
        printf("child\n");
        error = sem_post(sem1);
        if (error == FAIL) {
            error_exit("sem_post() failed: unable to unlock 1st semaphore", ERRNO_SET);
        }
    }
    my_sem_close(&sem1, &sem2);
    
    return NULL;
}

int main() {
    int pid = fork();
    if (pid == FAIL) {
        error_exit("fork() failed", ERRNO_SET);
    }
    if (pid == 0) {
        child_body();
    } else {
        sem_t *sem1, *sem2;
        my_sem_init(&sem1, &sem2);
        for (int i = 0; i < 10; i++) {
            int error = sem_wait(sem1);
            if (error == FAIL) {
                error_exit("sem_wait() failed: unable to lock 1st semaphore", ERRNO_SET);
            }
            printf("parent\n");
            error = sem_post(sem2);
            if (error == FAIL) {
                error_exit("sem_post() failed: unable to unlock 2nd semaphore", ERRNO_SET);
            }
        }
        my_sem_close(&sem1, &sem2);
    }
    
    pthread_exit(NULL);
}
