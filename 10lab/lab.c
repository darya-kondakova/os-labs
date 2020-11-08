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

pthread_mutex_t mutex[3];

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

    for (int i = 0; i < 3; i++) {
        error = pthread_mutex_init(&mutex[i], &mutex_attr);
        if (error != SUCCESS) {
            error_exit("pthread_mutex_init() failed", error);
        }
    }

    error = pthread_mutexattr_destroy(&mutex_attr);
    if (error != SUCCESS) {
        error_exit("pthread_mutexattr_destroy() failed", error);
    }
}

void mutex_destroy() {
    for (int i = 0; i < 3; i++) {
        int error = pthread_mutex_destroy(&mutex[i]);
        if (error != SUCCESS) {
            error_exit("pthread_mutex_destroy() failed", error);
        }
    }
}

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

void *child_body(void* arg) {
    my_pthread_mutex_lock(&mutex[2]);

    for (int i = 0; i < 10; i++) {
        my_pthread_mutex_lock(&mutex[0]);
        my_pthread_mutex_unlock(&mutex[2]);
        //my_pthread_mutex_lock(&mutex[1]);
        printf("child\n");
        my_pthread_mutex_unlock(&mutex[0]);
        my_pthread_mutex_lock(&mutex[2]);
        //my_pthread_mutex_unlock(&mutex[1]);
    }

    my_pthread_mutex_unlock(&mutex[2]);

    return NULL;
}

int main() {
    mutex_init();

    my_pthread_mutex_lock(&mutex[0]);
    //my_pthread_mutex_lock(&mutex[1]);

    pthread_t child;
    int error = pthread_create(&child, NULL, child_body, NULL);
    if (error != 0) {
        error_exit("pthread_create() failed", error);
    }

    //sleep(1);

    for (int i = 0; i < 10; i++) {
		sleep(1);
        printf("parent\n");
        my_pthread_mutex_unlock(&mutex[0]);
        my_pthread_mutex_lock(&mutex[2]);
        //my_pthread_mutex_unlock(&mutex[1]);
        my_pthread_mutex_lock(&mutex[0]);
        my_pthread_mutex_unlock(&mutex[2]);
        //my_pthread_mutex_lock(&mutex[1]);
    }

    my_pthread_mutex_unlock(&mutex[0]);
    //my_pthread_mutex_unlock(&mutex[1]);

    error = pthread_join(child, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_join() failed", error);
    }

    mutex_destroy();
    pthread_exit(NULL);
}
