#pragma once

#include <pthread.h>

void list_mutex_lock(pthread_mutex_t *mutex);

void list_mutex_unlock(pthread_mutex_t *mutex);

typedef struct List_Elem {
    char* string;
    struct List_Elem* next;
    pthread_mutex_t mutex;
} list_elem;

list_elem* create_list();

list_elem* create_elem(char* str);

void free_list(list_elem* l);

void print_list(list_elem* l);

void push_front(list_elem* l, char* string);

void sort(list_elem *l);
