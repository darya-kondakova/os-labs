#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"
#include "error_exit.h"

void init(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int error = pthread_cond_init(cond, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_cond_init() failed", error);
    }
    error = pthread_mutex_init(mutex, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
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

void my_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int error = pthread_cond_wait(cond, mutex);
    if (error != SUCCESS) {
        error_exit("pthread_cond_wait() failed", error);
    }
}

void my_pthread_cond_broadcast(pthread_cond_t *cond) {
    int error = pthread_cond_broadcast(cond);
    if (error != SUCCESS) {
        error_exit("pthread_cond_broadcast() failed", error);
    }
}

void destroy(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    int error = pthread_cond_destroy(cond);
    if (error != SUCCESS) {
        error_exit("pthread_cond_destroy() failed", error);
    }
    error = pthread_mutex_destroy(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }
}

Queue::Queue() {
    head = NULL;
    tail = NULL;
    droped = false;
    size = 0;
    init(&cond, &queue_mutex);
}

Queue::~Queue() {
    destroy(&cond, &queue_mutex);
}

void Queue::mymsgdrop() {
    /*drope нужно менять до вызова pthread_cond_broadcast(), т.к. нити сразу после выхода из pthread_cond_wait() 
    в функциях Queue::mymsgput() и Queue::mymsgget() должны знать, что была вызвана функция Queue::mymsgdrop().
    Это безопасно выполнять вне "защищённой секции", т.к. не испортит работу других нитей: 
    если изменение drop произойдёт до её проверки другими нитями в get() и put(), то нити просто завершат выполнение этих функций, разблокировав мьютекс;
    если после - нити выполнят свою работу (добавят сообщение в очередь/заберут сообщение из очереди), при этом Queue::mymsgdrop() не сможет отчистить 
    очередь пока мьютекс не разблокируют. В дальнейшем нити будут выходить из get() и put() по значению переменной drop.
    Также никакая нить не сможет работать с очередью в get() и put(), пока мьютекс будет заблокирован в функции drop()*/
    droped = true;
    my_pthread_cond_broadcast(&cond);
    my_pthread_mutex_lock(&queue_mutex);
    struct queue_element *t = head;
    while (t) {
        struct queue_element *t1 = t->next;
        free(t);
        t = t1;
    }
    size = 0;
    my_pthread_mutex_unlock(&queue_mutex);
}

int Queue::mymsgput(char *msg) {
    my_pthread_mutex_lock(&queue_mutex);
    while (size >= 10 && !droped) {
        my_pthread_cond_wait(&cond, &queue_mutex);
    }
    if (droped) {
        my_pthread_mutex_unlock(&queue_mutex);
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
    my_pthread_cond_broadcast(&cond);
    my_pthread_mutex_unlock(&queue_mutex);
    return strlen(t->buf);
}

int Queue::mymsgget(char *buf, size_t bufsize) {
    my_pthread_mutex_lock(&queue_mutex);
    while (size <= 0 && !droped) {
        my_pthread_cond_wait(&cond, &queue_mutex);
    }
    if (droped) {
        my_pthread_mutex_unlock(&queue_mutex);
        return 0;
    }
    struct queue_element *t = head;
    if (tail == t) {
        head = tail = NULL;
    } else {
        head = t->next;
        head->prev = NULL;
    }
    my_pthread_mutex_unlock(&queue_mutex);
    strncpy(buf, t->buf, bufsize-1);
    buf[strlen(buf)] = '\0';
    free(t);
    size--;
    my_pthread_cond_broadcast(&cond);
    return strlen(buf);
}
