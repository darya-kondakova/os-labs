#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 20
#define SUCCESS 0
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

void my_pthread_mutex_lock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_lock(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_lock() failed", error);
    }
}

void my_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_unlock(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_unlock() failed", error);
    }
}

int food_on_table() {
    static int food = FOOD;

    my_pthread_mutex_lock(&foodlock);
    if (food > 0) {
        food--;
    }
    int myfood = food;
    my_pthread_mutex_unlock(&foodlock);

    return myfood;
}

void get_forks(int phil, int f1, int f2) {
    my_pthread_mutex_lock(&add_fork);
    for (;;) {
        if (!pthread_mutex_trylock(&forks[f1])) {
            if (!pthread_mutex_trylock(&forks[f2])) {
                my_pthread_mutex_unlock(&add_fork);
                printf("Philosopher %d: got forks %d-%d\n", phil, f1, f2);
                return;
            }
            my_pthread_mutex_unlock(&forks[f1]);
        }
        int error = pthread_cond_wait(&condition, &add_fork);
        if (error != SUCCESS) {
            error_exit("pthread_cond_wait() failed", error);
        }
    }
}

void down_forks(int f1, int f2) {
    my_pthread_mutex_lock(&add_fork);
    my_pthread_mutex_unlock(&forks[f1]);
    my_pthread_mutex_unlock(&forks[f2]);
    int error = pthread_cond_broadcast(&condition);
    if (error != SUCCESS) {
        error_exit("pthread_cond_broadcast() failed", error);
    }
    my_pthread_mutex_unlock(&add_fork);
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
    int error = pthread_mutex_init(&foodlock, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }
    for (int i = 0; i < PHILO; i++) {
        error = pthread_mutex_init(&forks[i], NULL);
        if (error != SUCCESS) {
            error_exit("pthread_mutex_init() failed", error);
        }
    }
    pthread_t phils[PHILO];
    for (int i = 0; i < PHILO; i++) {
        error = pthread_create(&phils[i], NULL, philosopher, (void *)((long)i));
        if (error != SUCCESS) {
            error_exit("pthread_create() failed", error);
        }
    }
    for (int i = 0; i < PHILO; i++) {
        error = pthread_join(phils[i], NULL);
        if (error != SUCCESS) {
            error_exit("pthread_join() failed", error);
        }
    }
    for (int i = 0; i < PHILO; i++) {
        error = pthread_mutex_destroy(&forks[i]);
        if (error != SUCCESS) {
            error_exit("pthread_mutex_destroy() failed", error);
        }
    }

    pthread_exit(NULL);
}
