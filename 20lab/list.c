#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

list_elem* create_list() {
    list_elem *head = (list_elem *)malloc(sizeof(list_elem));
    if (head == NULL) {
        error_exit("malloc() failed", ERRNO_SET);
    }

    head->next = NULL;
    int error = pthread_rwlock_init(&head->lock, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_init() failed", error);
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
    int error = pthread_rwlock_init(&elem->lock, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_init() failed", error);
    }
    return elem;
}

void free_list(list_elem* head)  {
    list_rwlock_wrlock(&head->lock);
    list_elem* cur = head->next;
    while (cur) {
        list_elem* tmp = cur;
        cur = tmp->next;
        int error = pthread_rwlock_destroy(&tmp->lock);
        if (error != SUCCESS) {
            error_exit("pthread_rwlock_init() failed", error);
        }
        free(tmp->string);
        free(tmp);
    }
    list_rwlock_unlock(&head->lock);
    int error = pthread_rwlock_destroy(&head->lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_init() failed", error);
    }
    free(head);
}

void print_list(list_elem* head) {
    list_elem *prev = head;
    list_rwlock_rdlock(&prev->lock);
    list_elem* cur = prev->next;
    while (cur) {
        list_rwlock_rdlock(&cur->lock);
        printf("%s\n", cur->string);
        list_rwlock_unlock(&prev->lock);
        prev = cur;
        cur = cur->next;
    }
    list_rwlock_unlock(&prev->lock);
}

void push_front(list_elem* head, char* string) {
    list_elem *elem = create_elem(string);

    list_rwlock_wrlock(&head->lock);
    elem->next = head->next;
    head->next = elem;
    list_rwlock_unlock(&head->lock);
}

void sort(list_elem *head) {
    bool list_is_sorted = false;
    while (!list_is_sorted) {
        list_is_sorted = true;
        list_elem *prev = head;
        list_rwlock_wrlock(&prev->lock);
        list_elem *cur = prev->next;
        if (cur) {
            list_rwlock_wrlock(&cur->lock);
            list_elem *foll = cur->next;
            while (foll) {
                list_rwlock_wrlock(&foll->lock);
                if (0 < strcmp(cur->string, foll->string)) {
                    list_is_sorted = false;
                    prev->next = foll;
                    cur->next = foll->next;
                    foll->next = cur;
                    cur = foll;
                    foll = foll->next;
                }
                list_rwlock_unlock(&prev->lock);
                prev = cur;
                cur = foll;
                foll = foll->next;
                list_rwlock_unlock(&prev->lock);
                list_rwlock_unlock(&cur->lock);
                sleep(1);
                list_rwlock_wrlock(&prev->lock);
                list_rwlock_wrlock(&cur->lock);
            }
            list_rwlock_unlock(&cur->lock);
        }
        list_rwlock_unlock(&prev->lock);
    }
}
