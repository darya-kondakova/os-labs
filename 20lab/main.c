#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include "list.h"
#include "error_exit.h"

#define STR_SIZE 4
bool interrupt = false;

void exit_sig() {
    interrupt = true;
}

void *sort_list(void* arg) {
    list_elem *l = (list_elem *)arg;
	for (;;) {
		sleep(5);
        sort(l);
	}
}

void read_push(list_elem *list) {
    char str[STR_SIZE+1];
    while (!interrupt) {
        printf("Enter the string: ");
        bool new_line = true;
        do {
            fgets(str, STR_SIZE+1, stdin);
            if (str[0] == '\n' && new_line) {
                print_list(list);
                continue;
            }
            new_line = false;
            if (str[strlen(str) - 1] == '\n') {
                str[strlen(str) - 1] = '\0';
                new_line = true;
            }
            if (strlen(str) > 0) {
                push_front(list, str);
            }
        } while (!new_line);
    }
}

int main() {
    sigset(SIGINT, exit_sig);

    list_elem *list = create_list();

    pthread_t sorting_thread;
	int error = pthread_create(&sorting_thread, NULL, sort_list, list);
	if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
	}
	
    read_push(list);

    error = pthread_cancel(sorting_thread);
    if (error != SUCCESS) {
        error_exit("pthread_cancel() failed", error);
    }
    error = pthread_join(sorting_thread, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_join() failed", error);
    }

    free_list(list);
}
