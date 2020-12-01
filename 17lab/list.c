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

list_elem* create_list() {
    list_elem *head = (list_elem *)malloc(sizeof(list_elem));
    if (head == NULL) {
        error_exit("malloc() failed", ERRNO_SET);
    }

    head->next = NULL;
    int error = pthread_mutex_init(&head->mutex, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }
    return head;
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
    int error = pthread_mutex_init(&elem->mutex, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }
    return elem;
}

void free_list(list_elem* head)  {
    list_mutex_lock(&head->mutex);
    list_elem* cur = head->next;
    while (cur) {
        list_elem* tmp = cur;
        cur = tmp->next;
        int error = pthread_mutex_destroy(&tmp->mutex);
        if (error != SUCCESS) {
            error_exit("pthread_mutex_init() failed", error);
        }
        free(tmp->string);
        free(tmp);
    }
    list_mutex_unlock(&head->mutex);
    int error = pthread_mutex_destroy(&head->mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }
    free(head);
}

void print_list(list_elem* head) {
    list_elem *prev = head;
    list_mutex_lock(&prev->mutex);
    list_elem* cur = prev->next;
    while (cur) {
        list_mutex_lock(&cur->mutex);
        printf("%s\n", cur->string);
        list_mutex_unlock(&prev->mutex);
        prev = cur;
        cur = cur->next;
    }
    list_mutex_unlock(&prev->mutex);
}

void push_front(list_elem* head, char* string) {
    list_elem *elem = create_elem(string);

    list_mutex_lock(&head->mutex);
    elem->next = head->next;
    head->next = elem;
    list_mutex_unlock(&head->mutex);
}

void sort(list_elem *head) {
    bool list_is_sorted = false;
    while (!list_is_sorted) {
        list_is_sorted = true;
        list_elem *prev = head;
        list_mutex_lock(&prev->mutex);
        list_elem *cur = prev->next;
        if (cur) {
            list_mutex_lock(&cur->mutex);
            list_elem *foll = cur->next;
            while (foll) {
                list_mutex_lock(&foll->mutex);
                if (0 < strcmp(cur->string, foll->string)) {
                    list_is_sorted = false;
                    prev->next = foll;
                    cur->next = foll->next;
                    foll->next = cur;
                    cur = foll;
                    foll = foll->next;
                }
                list_mutex_unlock(&prev->mutex);
                prev = cur;
                cur = foll;
                foll = foll->next;
            }
            list_mutex_unlock(&cur->mutex);
        }
        list_mutex_unlock(&prev->mutex);
    }
}
