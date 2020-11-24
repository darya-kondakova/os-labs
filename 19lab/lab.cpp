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
#define STR_SIZE 80
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
pthread_rwlock_t list_lock;

void exit_sig(int sig) {
    int error = pthread_cancel(sorting_thread);
    if (error != SUCCESS) {
        error_exit("pthread_cancel() failed", error);
    }
    error = pthread_join(sorting_thread, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_join() failed", error);
    }

    error = pthread_rwlock_destroy(&list_lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_destroy() failed", error);
    }

    pthread_exit(NULL);
}

void my_pthread_rwlock_wrlock(pthread_rwlock_t *lock) {
    int error = pthread_rwlock_wrlock(lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_wrlock() failed", error);
    }
}

void my_pthread_rwlock_rdlock(pthread_rwlock_t *lock) {
    int error = pthread_rwlock_rdlock(lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_rdlock() failed", error);
    }
}

void my_pthread_rwlock_unlock(pthread_rwlock_t *lock) {
    int error = pthread_rwlock_unlock(lock);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_unlock() failed", error);
    }
}

void *sort_list(void* arg) {
    for (;;) {
        sleep(5);
        my_pthread_rwlock_rdlock(&list_lock);
        for (auto i = 0; i < l.size(); i++) {
            bool list_is_sorted = true;
            for (auto it1 = l.begin(); it1 != prev(l.end()); it1++) {
                auto it2 = next(it1);
                if (*it1 > *it2) {
                    if (list_is_sorted) {
                        my_pthread_rwlock_unlock(&list_lock);
                        my_pthread_rwlock_wrlock(&list_lock);
                    }
                    list_is_sorted = false;
                    swap(*it1, *it2);
                }
            }
            if (list_is_sorted) {
                break;
            }
        }
        my_pthread_rwlock_unlock(&list_lock);
    }
}

int main() {
    sigset(SIGINT, exit_sig);

    int error = pthread_rwlock_init(&list_lock, NULL);
    if (error != SUCCESS) {
        error_exit("pthread_rwlock_init() failed", error);
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
            my_pthread_rwlock_rdlock(&list_lock);
            for (string n : l) {
                cout << n << endl;
            }
            my_pthread_rwlock_unlock(&list_lock);
            continue;
        }
        for (auto i = 0; i < input_str.length(); i += STR_SIZE) {
            my_pthread_rwlock_wrlock(&list_lock);
            l.push_front(input_str.substr(i, STR_SIZE));
            my_pthread_rwlock_unlock(&list_lock);
        }
    }
}
