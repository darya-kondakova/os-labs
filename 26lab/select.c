#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAXLINES 25
#define SUCCESS 0
#define FAIL -1
#define ERRNO_SET 0

int open_socket(char *hostname, int port);

char *generate_get_command(const char *url, char *buf, size_t size);

int client_body(char *hostname, int port, char *url) {
    char buf[4096];
    int lines = 0;
    int lowmark = 0, highmark = 0;
    bool empty = true, full = false, eof = false;

    int socket_fd = open_socket(hostname, port);

    generate_get_command(url, buf, sizeof(buf));
    write(socket_fd, buf, strlen(buf));
    for (;;) {
        fd_set readfs, writefs;
        FD_ZERO(&readfs);
        FD_ZERO(&writefs);
        if (!full && !eof) {
            FD_SET(socket_fd, &readfs);
        }
        if (!empty && lines < MAXLINES) {
            FD_SET(STDOUT_FILENO, &writefs);
        }
        if (lines >= MAXLINES) {
            FD_SET(STDIN_FILENO, &readfs);
        }

        int res = select(socket_fd + 1, &readfs, &writefs, NULL, NULL);
        if (res == FAIL) {
            perror("select() failed");
        }
        if (res == 0) {
            continue;
        }

        if (!full && !eof && FD_ISSET(socket_fd, &readfs)) {
            if (highmark >= lowmark) {
                res = read(socket_fd, buf + highmark, sizeof(buf) - highmark);
                if (res == 0) {
                    eof = true;
                } else {
                    highmark += res;
                    if (highmark == sizeof(buf)) {
                        highmark = 0;
                    }
                    if (highmark == lowmark) {
                        full = true;
                    }
                    empty = false;
                }
            } else {
                res = read(socket_fd, buf + highmark, lowmark - highmark);
                if (res == 0) {
                    eof = true;
                } else {
                    highmark += res;
                    if (highmark == lowmark) {
                        full = true;
                    }
                    empty = false;
                }
            }
        }
        if (!empty && lines < 25 && FD_ISSET(STDOUT_FILENO, &writefs)) {
            char *t = buf + lowmark;
            char *l = (lowmark < highmark) ? buf + highmark : buf + sizeof(buf);

            for (; t < l && *t != '\n'; t++);
            if (t < l) {
                t++;
                lines++;
            }
            res = write(STDOUT_FILENO, buf + lowmark, t - (buf + lowmark));
            if (res == FAIL) {
                perror("write() failed");
            }
            lowmark += res;
            full = false;
            if (lowmark == sizeof(buf)) {
                lowmark = 0;
            }
            if (lowmark == highmark) {
                empty = true;
            }
        }

        if (eof && empty) {
            break;
        }

        if (lines >= 25 && FD_ISSET(STDIN_FILENO, &readfs)) {
            char tb[1];
            read(STDIN_FILENO, tb, 1);
            lines = 0;
        }
    }
    return 0;
}
