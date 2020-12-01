#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"
#include "error_exit.h"

void list_mutex_lock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_lock(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_lock() failed", error);
    }
}

void list_mutex_unlock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_unlock(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_unlock() failed", error);
    }
}

list_t* create_list() {
    list_t *list = (list_t *)malloc(sizeof(list_t));
    if (list == NULL) {
        error_exit("malloc() failed", ERRNO_SET);
    }

    list->head = NULL;
    list->size = 0;
    int error = pthread_mutex_init(&list->mutex, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }
    return list;
}

list_elem* create_elem(char* str) {
    list_elem *elem = (list_elem *)malloc(sizeof(list_elem));
    if (elem == NULL) {
        error_exit("malloc() failed", ERRNO_SET);
    }

    const size_t len = strlen(str) + 1;
    elem->string = (char *)malloc(len);
    if (elem->string == NULL) {
        error_exit("malloc() failed", ERRNO_SET);
    }
    strncpy(elem->string, str, len);
    elem->next = NULL;
    return elem;
}

void free_list(list_t* l)  {
    list_mutex_lock(&l->mutex);
    list_elem* cur = l->head;
    while (cur) {
         list_elem* tmp = cur;
         cur = tmp->next;
         free(tmp->string);
         free(tmp);
    }
    list_mutex_unlock(&l->mutex);
    pthread_mutex_destroy(&l->mutex);
    free(l);
}

void print_list(list_t* l)  {
    list_mutex_lock(&l->mutex);
    list_elem* cur = l->head;
    while (cur) {
        printf("%s\n", cur->string);
        cur = cur->next;
    }
    list_mutex_unlock(&l->mutex);
}

void push_front(list_t* l, char* string) {
    list_elem *elem = create_elem(string);

    list_mutex_lock(&l->mutex);
    elem->next = l->head;
    l->head = elem;
    l->size++;
    list_mutex_unlock(&l->mutex);
}

void swap(list_elem* el1, list_elem* el2) {
    char* tmp = el1->string;
    el1->string = el2->string;
    el2->string = tmp;
}

void sort(list_t *l) {
    list_mutex_lock(&l->mutex);
    for (int i = 0; i < l->size; i++) {
        bool list_is_sorted = true;
        for (list_elem *el1 = l->head; el1->next != NULL; el1 = el1->next) {
            list_elem *el2 = el1->next;
            if (0 < strcmp(el1->string, el2->string)) {
                list_is_sorted = false;
                swap(el1, el2);
            }
        }
        if (list_is_sorted) {
            break;
        }
    }
    list_mutex_unlock(&l->mutex);
}
