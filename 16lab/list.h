#pragma once

#include <pthread.h>

void my_pthread_mutex_lock(pthread_mutex_t *mutex);

void my_pthread_mutex_unlock(pthread_mutex_t *mutex);

typedef struct List_Elem {
    char* string;
    struct List_Elem* next;
} list_elem;

typedef struct List {
    list_elem* head;
    int size;
    pthread_mutex_t mutex;
} list_t;

list_t* create_list();

list_elem* create_elem(char* str);

void free_list(list_t* l);

void print_list(list_t* l);

void push_front(list_t* l, char* string);

void swap(list_elem* el1, list_elem* el2);

void sort(list_t *l);
