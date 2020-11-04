#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

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

pthread_mutex_t mutex1;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

void mutex_init() {
    pthread_mutexattr_t mutex_attr;
    int error = pthread_mutexattr_init(&mutex_attr);
    if (error != SUCCESS) {
        error_exit("pthread_mutexattr_init() failed", error);
    }
    error = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    if (error != SUCCESS) {
        error_exit("pthread_mutexattr_settype() failed", error);
    }

    error = pthread_mutex_init(&mutex1, &mutex_attr);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }

    error = pthread_mutexattr_destroy(&mutex_attr);
    if (error != SUCCESS) {
        error_exit("pthread_mutexattr_destroy() failed", error);
    }
}

void my_pthread_mutex_lock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_lock(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_lock() failed", error);
    }
}

void print_str(char *str) {
    int error;
    for (int i = 0; i < 10; i++) {
        printf("%s\n", str);

        error = pthread_cond_signal(&condition);
        if (error != SUCCESS) {
            error_exit("pthread_cond_signal() failed", error);
        }

        error = pthread_cond_wait(&condition, &mutex1);
        if (error != SUCCESS) {
            error_exit("pthread_cond_wait() failed", error);
        }
    }

    error = pthread_mutex_unlock(&mutex1);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_unlock() failed", error);
    }
}

void *child_body(void* arg) {
    my_pthread_mutex_lock(&mutex1);

    print_str("child");

    return NULL;
}

int main() {
    mutex_init();
    my_pthread_mutex_lock(&mutex1);

    pthread_t child;
    int error = pthread_create(&child, NULL, child_body, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }

    print_str("parent");

    error = pthread_cond_signal(&condition);
    if (error != SUCCESS) {
        error_exit("pthread_cond_signal() failed", error);
    }

    error = pthread_join(child, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_join() failed", error);
    }

    error = pthread_mutex_destroy(&mutex1);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_destroy() failed", error);
    }

    pthread_exit(NULL);
}
