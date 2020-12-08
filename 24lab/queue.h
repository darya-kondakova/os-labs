#pragma once
#include <pthread.h>

typedef struct queue_element {
  struct queue_element *next, *prev;
  char buf[81];
}q_elem;

class Queue {
private:
    struct queue_element *head, *tail;
    pthread_cond_t cond;
    pthread_mutex_t cond_mutex, queue_mutex;
    int size;
    bool droped;

public:
    Queue();
    ~Queue();

    void mymsgdrop();
    int mymsgput(char *msg);
    int mymsgget(char *buf, size_t bufsize);
};
