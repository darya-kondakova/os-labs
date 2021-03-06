#pragma once

#include <pthread.h>

void list_rwlock_wrlock(pthread_rwlock_t *lock);

void list_rwlock_rdlock(pthread_rwlock_t *lock);

void list_rwlock_unlock(pthread_rwlock_t *lock);

typedef struct List_Elem {
    char* string;
    struct List_Elem* next;
} list_elem;

typedef struct List {
    list_elem* head;
    int size;
    pthread_rwlock_t lock;
} list_t;

list_t* create_list();

list_elem* create_elem(char* str);

void free_list(list_t* l);

void print_list(list_t* l);

void push_front(list_t* l, char* string);

void swap(list_elem* el1, list_elem* el2);

void sort(list_t *l);
