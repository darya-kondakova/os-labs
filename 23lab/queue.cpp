#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "queue.h"

Queue::Queue() {
    head = NULL;
    tail = NULL;
    droped = false;
    sem_init(&put_sem, 0, 10);
    sem_init(&get_sem, 0, 0);
    sem_init(&queue_sem, 0, 1);
}

Queue::~Queue() {
    sem_destroy(&put_sem);
    sem_destroy(&get_sem);
    sem_destroy(&queue_sem);
}

void Queue::mymsgdrop() {
    droped = true;
    sem_wait(&queue_sem);
    sem_post(&put_sem);
    sem_post(&get_sem);
    struct queue_element *t = head;
    while (t) {
        struct queue_element *t1 = t->next;
        free(t);
        t = t1;
    }
    sem_post(&queue_sem);
}

int Queue::mymsgput(char *msg) {
    sem_wait(&put_sem);
    sem_wait(&queue_sem);
    if (droped) {
        sem_post(&put_sem);
        sem_post(&queue_sem);
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
    sem_post(&get_sem);
    sem_post(&queue_sem);
    return strlen(t->buf);
}

int Queue::mymsgget(char *buf, size_t bufsize) {
    sem_wait(&get_sem);
    sem_wait(&queue_sem);
    if (droped) {
        sem_post(&get_sem);
        sem_post(&queue_sem);
        return 0;
    }
    struct queue_element *t = head;
    if (tail == t) {
        head = tail = NULL;
    } else {
        head = t->next;
        head->prev = NULL;
    }
    sem_post(&queue_sem);
    strncpy(buf, t->buf, bufsize-1);
    buf[strlen(buf)] = '\0';
    free(t);
    sem_post(&put_sem);
    return strlen(buf);
}
