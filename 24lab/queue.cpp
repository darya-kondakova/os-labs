#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

Queue::Queue() {
    head = NULL;
    tail = NULL;
    droped = false;
    size = 0;
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
}

Queue::~Queue() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&queue_mutex);
}

void Queue::mymsgdrop() {
    droped = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_lock(&queue_mutex);
    struct queue_element *t = head;
    while (t) {
        struct queue_element *t1 = t->next;
        free(t);
        t = t1;
    }
    size = 0;
    pthread_mutex_unlock(&queue_mutex);
}

int Queue::mymsgput(char *msg) {
    pthread_mutex_lock(&queue_mutex);
    while (size >= 10 && !droped) {
        pthread_cond_wait(&cond, &queue_mutex);
    }
    if (droped) {
        pthread_mutex_unlock(&queue_mutex);
        return 0;
    }
    q_elem *t = (q_elem*)malloc(sizeof(q_elem));
    strncpy(t->buf, msg, sizeof(t->buf)-1);
    t->buf[strlen(t->buf)] = '\0';
    t->prev = tail;
    t->next = NULL;
    if (tail == NULL) {
        head = tail = t;
    } else {
        tail->next = t;
        tail = t;
    }
    size++;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&queue_mutex);
    return strlen(t->buf);
}

int Queue::mymsgget(char *buf, size_t bufsize) {
    pthread_mutex_lock(&queue_mutex);
    while (size <= 0 && !droped) {
        pthread_cond_wait(&cond, &queue_mutex);
    }
    if (droped) {
        pthread_mutex_unlock(&queue_mutex);
        return 0;
    }
    struct queue_element *t = head;
    if (tail == t) {
        head = tail = NULL;
    } else {
        head = t->next;
        head->prev = NULL;
    }
    pthread_mutex_unlock(&queue_mutex);
    strncpy(buf, t->buf, bufsize-1);
    buf[strlen(buf)] = '\0';
    free(t);
    size--;
    pthread_cond_broadcast(&cond);
    return strlen(buf);
}
