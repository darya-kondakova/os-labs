#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50
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

int food_on_table() {
    static int food = FOOD;

    pthread_mutex_lock(&foodlock);
    if (food > 0) {
        food--;
    }
    int myfood = food;
    pthread_mutex_unlock(&foodlock);

    return myfood;
}

void get_fork(int phil, int fork, char *hand) {
    pthread_mutex_lock(&forks[fork]);
    printf("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void down_forks(int f1, int f2) {
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
}

void *philosopher(void *num) {
    int id = (int)num;
    printf("Philosopher %d sitting down to dinner.\n", id);
    int left_fork = id,
    	right_fork = (id + 1) % 5; /* Wrap around the forks. */

	int f;
    while ((f = food_on_table())) {
		printf("Philosopher %d: reflects.\n", id);
		usleep(DELAY * (FOOD - f + 1));

        printf("Philosopher %d: get dish %d.\n", id, f);
        get_fork(id, left_fork, "left ");
        get_fork(id, right_fork, "right");

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }
    printf("Philosopher %d is done eating.\n", id);

    return NULL;
}

int main(int argn, char **argv) {
	int error = pthread_mutex_init(&foodlock, NULL);
    if (error != 0) {
		error_exit("pthread_mutex_init() failed", error);
	}
    for (int i = 0; i < PHILO; i++) {
		error = pthread_mutex_init(&forks[i], NULL); 
        if (error != 0) {
			error_exit("pthread_mutex_init() failed", error);
		}
	}
	pthread_t phils[PHILO];
    for (int i = 0; i < PHILO; i++) {
		error = pthread_create(&phils[i], NULL, philosopher, (void *)((long)i));
    	if (error != 0) {
			error_exit("pthread_create() failed", error);
		}
	}
    for (int i = 0; i < PHILO; i++) {
        error = pthread_join(phils[i], NULL);
		if (error != 0) {
			error_exit("pthread_join() failed", error);
		}
	}
	for (int i = 0; i < PHILO; i++) {
        error = pthread_mutex_destroy(&forks[i]);
        if (error != 0) {
            error_exit("pthread_mutex_destroy() failed", error);
        }
    }

    return NULL;
}
