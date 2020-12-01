#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"
#include "error_exit.h"

void list_rwlock_wrlock(pthread_rwlock_t *lock) {
    int error = pthread_rwlock_wrlock(lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_wrlock() failed", error);
    }
}

void list_rwlock_rdlock(pthread_rwlock_t *lock) {
    int error = pthread_rwlock_rdlock(lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_rdlock() failed", error);
    }
}

void list_rwlock_unlock(pthread_rwlock_t *lock) {
    int error = pthread_rwlock_unlock(lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_unlock() failed", error);
    }
}

list_t* create_list() {
    list_t *list = (list_t *)malloc(sizeof(list_t));
    if (list == NULL) {
        error_exit("malloc() failed", ERRNO_SET);
    }

    list->head = NULL;
    list->size = 0;
    int error = pthread_rwlock_init(&list->lock, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_init() failed", error);
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
    list_rwlock_wrlock(&l->lock);
    list_elem* cur = l->head;
    while (cur) {
         list_elem* tmp = cur;
         cur = tmp->next;
         free(tmp->string);
         free(tmp);
    }
    list_rwlock_unlock(&l->lock);
    int error = pthread_rwlock_destroy(&l->lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_destroy() failed", error);
    }
    free(l);
}

void print_list(list_t* l)  {
    list_rwlock_rdlock(&l->lock);
    list_elem* cur = l->head;
    while (cur) {
        printf("%s\n", cur->string);
        cur = cur->next;
    }
    list_rwlock_unlock(&l->lock);
}

void push_front(list_t* l, char* string) {
    list_elem *elem = create_elem(string);

    list_rwlock_wrlock(&l->lock);
    elem->next = l->head;
    l->head = elem;
    l->size++;
    list_rwlock_unlock(&l->lock);
}

void swap(list_elem* el1, list_elem* el2) {
    char* tmp = el1->string;
    el1->string = el2->string;
    el2->string = tmp;
}

void sort(list_t *l) {
    list_rwlock_rdlock(&l->lock);
    for (int i = 0; i < l->size; i++) {
        bool list_is_sorted = true;
        for (list_elem *el1 = l->head; el1->next != NULL; el1 = el1->next) {
            list_elem *el2 = el1->next;
            if (0 < strcmp(el1->string, el2->string)) {
                if (list_is_sorted) {
                    list_rwlock_unlock(&l->lock);
                    list_rwlock_wrlock(&l->lock);
                }
                list_is_sorted = false;
                swap(el1, el2);
            }
        }
        if (list_is_sorted) {
            break;
        }
    }
    list_rwlock_unlock(&l->lock);
}
