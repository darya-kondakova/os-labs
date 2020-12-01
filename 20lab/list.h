#pragma once

#include <pthread.h>

void list_rwlock_wrlock(pthread_rwlock_t *lock);

void list_rwlock_rdlock(pthread_rwlock_t *lock);

void list_rwlock_unlock(pthread_rwlock_t *lock);

typedef struct List_Elem {
    char* string;
    struct List_Elem* next;
    pthread_rwlock_t lock;
} list_elem;

list_elem* create_list();

list_elem* create_elem(char* str);

void free_list(list_elem* l);

void print_list(list_elem* l);

void push_front(list_elem* l, char* string);

void sort(list_elem *l);
