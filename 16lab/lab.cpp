#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <list>
#include <algorithm>
#include <iterator>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

using namespace std;

#define min(a, b) a < b ? a : b
#define STR_SIZE 4
#define SUCCESS 0
#define ERRNO_SET 0
#define ERROR_EXIT 1

void error_exit(const char *const msg, int error) {
    if (error != ERRNO_SET) {
        cerr << msg << strerror(error);
    } else {
        if (errno == 0) {
            cerr << msg << endl;
        } else {
            perror(msg);
        }
    }
    exit(ERROR_EXIT);
}

list<string> l = {};
pthread_t sorting_thread;
pthread_mutex_t list_mutex;

void exit_sig(int sig) {
    int error = pthread_cancel(sorting_thread);
    if (error != SUCCESS) {
        error_exit("pthread_cancel() failed", error);
    }
    error = pthread_join(sorting_thread, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_join() failed", error);
    }

    error = pthread_mutex_destroy(&list_mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_destroy() failed", error);
    }

    pthread_exit(NULL);
}

void my_pthread_mutex_lock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_lock(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_lock() failed", error);
    }
}

void my_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    int error = pthread_mutex_unlock(mutex);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_unlock() failed", error);
    }
}

void *sort_list(void* arg) {
    for (;;) {
        sleep(5);
        my_pthread_mutex_lock(&list_mutex);
        for (int i = 0; i < l.size(); i++) {
            bool list_is_sorted = true;
            for (auto it1 = l.begin(); it1 != prev(l.end()); it1++) {
                auto it2 = next(it1);
                if (*it1 > *it2) {
                    list_is_sorted = false;
                    swap(*it1, *it2);
                }
            }
            if (list_is_sorted) {
                break;
            }
        }
        my_pthread_mutex_unlock(&list_mutex);
    }
}

int main() {
    sigset(SIGINT, exit_sig);

    int error = pthread_mutex_init(&list_mutex, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_mutex_init() failed", error);
    }

    error = pthread_create(&sorting_thread, NULL, sort_list, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_create() failed", error);
    }

    string input_str;
    for (;;) {
        cout << "Enter the string: ";
        getline(cin, input_str);
        if (input_str.empty()) {
            my_pthread_mutex_lock(&list_mutex);
            for (string n : l) {
                cout << n << endl;
            }
            my_pthread_mutex_unlock(&list_mutex);
            continue;
        }
        for (int i = 0; i <= (input_str.length() - 1) / STR_SIZE; i++) {
            string str(input_str, i * STR_SIZE, min(input_str.length() - i * STR_SIZE, STR_SIZE));
            my_pthread_mutex_lock(&list_mutex);
            l.push_front(str);
            my_pthread_mutex_unlock(&list_mutex);
        }
    }
}
