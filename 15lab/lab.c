#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

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
}

void my_sem_close(sem_t **sem1, sem_t **sem2) {
    sem_close(*sem1);
    sem_close(*sem2);
}

void child_body() {
    sem_t *sem1, *sem2;
    my_sem_init(&sem1, &sem2);
    for (int i = 0; i < 10; i++) {
        sem_wait(sem2);
        printf("child\n");
        sem_post(sem1);
    }
    my_sem_close(&sem1, &sem2);
}

int main() {
    int pid = fork();
    if (pid == -1) {
        error_exit("fork() failed", ERRNO_SET);
    }
    if (pid == 0) {
        child_body();
    } else {
        sem_t *sem1, *sem2;
        my_sem_init(&sem1, &sem2);
        for (int i = 0; i < 10; i++) {
            sem_wait(sem1);
            printf("parent\n");
            sem_post(sem2);
        }
        my_sem_close(&sem1, &sem2);
    }
}
