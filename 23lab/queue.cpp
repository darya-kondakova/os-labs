#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "queue.h"
#include "error_exit.h"

void my_sem_init(sem_t* put_sem, sem_t* get_sem, sem_t* queue_sem) {
    int error = sem_init(put_sem, 0, 10);
    if (error != SUCCESS) {
        error_exit("sem_init() failed", ERRNO_SET);
    }
    error = sem_init(get_sem, 0, 0);
    if (error != SUCCESS) {
        error_exit("sem_init() failed", ERRNO_SET);
    }
    error = sem_init(queue_sem, 0, 1);
    if (error != SUCCESS) {
        error_exit("sem_init() failed", ERRNO_SET);
    }
}

void my_sem_wait(sem_t* sem) {
    int error = sem_wait(sem);
    if (error != SUCCESS) {
        error_exit("sem_wait() failed", ERRNO_SET);
    }
}

void my_sem_post(sem_t* sem) {
    int error = sem_post(sem);
    if (error != SUCCESS) {
        error_exit("sem_post() failed", ERRNO_SET);
    }
}

void my_sem_destroy(sem_t* sem) {
    int error = sem_destroy(sem);
    if (error != SUCCESS) {
        error_exit("sem_destroy() failed", ERRNO_SET);
    }
}

Queue::Queue() {
    head = NULL;
    tail = NULL;
    droped = false;
    my_sem_init(&put_sem, &get_sem, &queue_sem);
}

Queue::~Queue() {
    my_sem_destroy(&put_sem);
    my_sem_destroy(&get_sem);
    my_sem_destroy(&queue_sem);
}

void Queue::mymsgdrop() {
    my_sem_wait(&queue_sem);
    droped = true;
    my_sem_post(&put_sem);
    my_sem_post(&get_sem);
    struct queue_element *t = head;
    while (t) {
        struct queue_element *t1 = t->next;
        free(t);
        t = t1;
    }
    my_sem_post(&queue_sem);
}

int Queue::mymsgput(char *msg) {
    my_sem_wait(&put_sem);
    my_sem_wait(&queue_sem);
    if (droped) {
        my_sem_post(&put_sem);
        my_sem_post(&queue_sem);
        return 0;
    }
    q_elem *t = (q_elem *) malloc(sizeof(q_elem));
    strncpy(t->buf, msg, sizeof(t->buf) - 1);
    t->buf[strlen(t->buf)] = '\0';
    t->prev = tail;
    t->next = NULL;
    if (tail == NULL) {
        head = tail = t;
    } else {
        tail->next = t;
        tail = t;
    }
    my_sem_post(&get_sem);
    my_sem_post(&queue_sem);
    return strlen(t->buf);
}

int Queue::mymsgget(char *buf, size_t bufsize) {
    my_sem_wait(&get_sem);
    my_sem_wait(&queue_sem);
    if (droped) {
        my_sem_post(&get_sem);
        my_sem_post(&queue_sem);
        return 0;
    }
    struct queue_element *t = head;
    if (tail == t) {
        head = tail = NULL;
    } else {
        head = t->next;
        head->prev = NULL;
    }
    my_sem_post(&queue_sem);
    strncpy(buf, t->buf, bufsize - 1);
    buf[strlen(buf)] = '\0';
    free(t);
    my_sem_post(&put_sem);
    return strlen(buf);
}

