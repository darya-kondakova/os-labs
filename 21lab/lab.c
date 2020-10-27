#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "error_exit.h"

#define PHILO 5
#define DELAY 30000
#define FOOD 20
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

pthread_mutex_t forks[PHILO];
pthread_mutex_t foodlock;
pthread_mutex_t add_fork;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

int food_on_table() {
    static int food = FOOD;
    int myfood;

    pthread_mutex_lock(&foodlock);
    if (food > 0) {
        food--;
    }
    myfood = food;
    pthread_mutex_unlock(&foodlock);
    return myfood;
}

void get_forks(int phil, int f1, int f2) {
    pthread_mutex_lock(&add_fork);
	for (;;) {
		if (!pthread_mutex_trylock(&forks[f1])) {
			if (!pthread_mutex_trylock(&forks[f2])) {
				pthread_mutex_unlock(&add_fork);
				printf("Philosopher %d: got forks %d-%d\n", phil, f1, f2);
				return;
			}
			pthread_mutex_unlock(&forks[f1]);
		}
		pthread_cond_wait(&condition, &add_fork);
	}
}

void down_forks(int f1, int f2) {
	pthread_mutex_lock(&add_fork);
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
	pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&add_fork);
}

void * philosopher(void *num) {
    int id = (int)num;
    printf("Philosopher %d sitting down to dinner.\n", id);
	int left_fork = id,
        right_fork = (id + 1) % 5; /* Wrap around the forks. */

	int f;
    while ((f = food_on_table())) {
        printf("Philosopher %d: get dish %d.\n", id, f);
        get_forks(id, left_fork, right_fork);

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }
    printf("Philosopher %d is done eating.\n", id);
    return (NULL);
}

int main(int argn, char **argv) {
	int error;
    if ((error = pthread_mutex_init(&foodlock, NULL)) != 0) {
        error_exit("pthread_mutex_init() failed", error);
    }
    for (int i = 0; i < PHILO; i++) {
        if ((error = pthread_mutex_init(&forks[i], NULL)) != 0) {
            error_exit("pthread_mutex_init() failed", error);
        }
    }
    pthread_t phils[PHILO];
    for (int i = 0; i < PHILO; i++) {
        if ((error = pthread_create(&phils[i], NULL, philosopher, (void *)((long)i))) != 0) {
            error_exit("pthread_create() failed", error);
        }
    }
    /*for (int i = 0; i < PHILO; i++)
        pthread_join(phils[i], NULL);
    return 0;*/
    pthread_exit(NULL);
}
