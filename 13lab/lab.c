#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>

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

sem_t sem1;
sem_t sem2;

void my_sem_init() {
	if (sem_init(&sem1, 0, 1) == -1) {
		error_exit("sem_init() failed: unable to initialize 1st semaphore", ERRNO_SET);
	}
	if (sem_init(&sem2, 0, 0) == -1) {
		error_exit("sem_init() failed: unable to initialize 2nd semaphore", ERRNO_SET);
	}
}

void my_sem_destroy() {
	if (sem_destroy(&sem1) == -1) {
		error_exit("sem_destroy() failed: unable to destroy 1st semaphore", ERRNO_SET);
	}
	if (sem_destroy(&sem2) == -1) {
		error_exit("sem_destroy() failed: unable to destroy 2nd semaphore", ERRNO_SET);
	}
}

void *child_body(void* arg) {
	for (int i = 0; i < 10; i++) {
		if (sem_wait(&sem2) == -1) {
			error_exit("sem_wait() failed: unable to lock 2nd semaphore", ERRNO_SET);
		}	
		printf("child\n");
		if (sem_post(&sem1) == -1) {
			error_exit("sem_post() failed: unable to unlock 1st semaphore", ERRNO_SET);
		}
	}

	return NULL;
}

int main() {
	int error;
	my_sem_init();

	pthread_t child;
	if ((error = pthread_create(&child, NULL, child_body, NULL)) != 0) {
		error_exit("pthread_create() failed", error);
	}
	
	for (int i = 0; i < 10; i++) {
		if (sem_wait(&sem1) == -1) {
			error_exit("sem_wait() failed: unable to lock 1st semaphore", ERRNO_SET);
		}	
        printf("parent\n");
		if (sem_post(&sem2) == -1) {
			error_exit("sem_post() failed: unable to unlock 2nd semaphore", ERRNO_SET);
		}
    }
	
	pthread_exit(NULL);
	my_sem_destroy();
}
