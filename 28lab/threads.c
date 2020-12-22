#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define MAXLINES 25
#define FAIL -1
#define SUCCESS 0

int open_socket(char *hostname, int port);

char *generate_get_command(const char *url, char *buf, size_t size);

void my_pthread_mutex_lock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_lock(mutex);
    if (error != SUCCESS) {
        fprintf(stderr, "pthread_mutex_lock() failed: %s", strerror(error));
    }
}

void my_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_unlock(mutex);
    if (error != SUCCESS) {
        fprintf(stderr, "pthread_mutex_unlock() failed: %s", strerror(error));
    }
}

void my_pthread_join(pthread_t thread) {
    int error = pthread_join(thread, NULL);
    if (error != SUCCESS) {
        fprintf(stderr, "pthread_join() failed: %s", strerror(error));
    }
}

void my_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int error = pthread_cond_wait(cond, mutex);
    if (error != SUCCESS) {
        fprintf(stderr, "pthread_cond_wait() failed: %s", strerror(error));
    }
}

void my_pthread_cond_signal(pthread_cond_t *cond) {
    int error = pthread_cond_signal(cond);
    if (error != SUCCESS) {
        fprintf(stderr, "pthread_cond_signal() failed: %s", strerror(error));
    }
}

int socket_fd;
char buf[4096];
int lines = 0;
int lowmark = 0, highmark = 0;
bool empty = true, full = false, eof = false;

pthread_mutex_t bufmx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bufcond = PTHREAD_COND_INITIALIZER;

void *reader_thread(void *arg) {
    int res;
    my_pthread_mutex_lock(&bufmx);
    while (!eof) {
        while (full) {
            my_pthread_cond_wait(&bufcond, &bufmx);
        }
        if (highmark >= lowmark) {
            my_pthread_mutex_unlock(&bufmx);
            res = read(socket_fd, buf + highmark, sizeof(buf) - highmark);
            my_pthread_mutex_lock(&bufmx);
            if (res == 0) {
                eof = true;
                break;
            }
            highmark += res;
            if (highmark == sizeof(buf)) {
                highmark = 0;
            }
        } else {
            int t = lowmark;
            my_pthread_mutex_unlock(&bufmx);
            res = read(socket_fd, buf + highmark, t - highmark);
            my_pthread_mutex_lock(&bufmx);
            if (res == 0) {
                eof = true;
                break;
            }
        }
        if (highmark == lowmark) {
            full = true;
        }
        empty = false;
        my_pthread_cond_signal(&bufcond);
    }
    my_pthread_mutex_unlock(&bufmx);
    return NULL;
}

void *writer_thread(void *arg) {
    int res;
    my_pthread_mutex_lock(&bufmx);
    while (!(eof && empty)) {
        char *t = buf + lowmark;
        char *l;

        while (empty) {
            my_pthread_cond_wait(&bufcond, &bufmx);
        }
        if (lowmark < highmark) {
            l = buf + highmark;
        } else {
            l = buf + sizeof(buf);
        }

        for (; t < l && *t != '\n'; t++);
        if (t < l) {
            t++;
            lines++;
        }
        my_pthread_mutex_unlock(&bufmx);
        if (lines >= MAXLINES) {
            char tb[10];
            read(STDIN_FILENO, tb, 1);
            lines = 0;
        }

        res = write(STDOUT_FILENO, buf + lowmark, t - (buf + lowmark));
        my_pthread_mutex_lock(&bufmx);
        if (res < 0) {
            perror("writing to termial");
            return NULL;
        }
        lowmark += res;
        if (lowmark >= sizeof(buf)) {
            lowmark = 0;
        }
        if (lowmark == highmark) {
            empty = true;
        }
        full = false;
        my_pthread_cond_signal(&bufcond);
    }
    my_pthread_mutex_unlock(&bufmx);
    return NULL;
}

void client_body(char *hostname, int port, char *url) {
    socket_fd = open_socket(hostname, port);

    generate_get_command(url, buf, sizeof(buf));
    write(socket_fd, buf, strlen(buf));

    pthread_t reader, writer;
    int res = pthread_create(&reader, NULL, reader_thread, NULL);
    if (res != SUCCESS) {
        fprintf(stderr, "pthread_create() failed: %s", strerror(res));
    }
    res = pthread_create(&writer, NULL, writer_thread, NULL);
    if (res != SUCCESS) {
        fprintf(stderr, "pthread_create() failed: %s", strerror(res));
    }

    my_pthread_join(reader);
    my_pthread_join(writer);
}
