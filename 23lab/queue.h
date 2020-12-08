#pragma once
#include <semaphore.h>

typedef struct queue_element {
  struct queue_element *next, *prev;
  char buf[81];
}q_elem;

class Queue {
private:
    struct queue_element *head, *tail;
    sem_t put_sem, get_sem, queue_sem;
    bool droped;

public:
    Queue();
    ~Queue();

    void mymsgdrop();
    int mymsgput(char *msg);
    int mymsgget(char *buf, size_t bufsize);
};
