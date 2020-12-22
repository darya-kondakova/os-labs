#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <aio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#define MAXLINES 25
#define FAIL -1
#define SUCCESS 0

int open_socket(char *hostname, int port);

char *generate_get_command(const char *url, char *buf, size_t size);

void client_body(char *hostname, int port, char *url) {
    char buf[4096];
    int lines = 0;
    int lowmark = 0, highmark = 0;
    bool empty = true, full = false, eof = false;
    int res;

    int socket_fd = open_socket(hostname, port);

    generate_get_command(url, buf, sizeof(buf));
    write(socket_fd, buf, strlen(buf));

    struct aiocb readrq, writerq, termreadrq;
    char tb[10];
    memset(&readrq, 0, sizeof(struct aiocb));
    readrq.aio_fildes = socket_fd;
    memset(&writerq, 0, sizeof writerq);
    writerq.aio_fildes = STDOUT_FILENO;
    memset(&termreadrq, 0, sizeof termreadrq);
    termreadrq.aio_fildes = STDIN_FILENO;
    termreadrq.aio_buf = tb;

    for (;;) {
        struct aiocb *rqv[4] = {NULL};
        memset(rqv, 0, sizeof(rqv));
        int rqt = 0;
        if (!full && !eof) {
            readrq.aio_buf = buf + highmark;
            if (highmark >= lowmark) {
                readrq.aio_nbytes = sizeof(buf) - highmark;
            } else {
                readrq.aio_nbytes = lowmark - highmark;
            }
            res = aio_read(&readrq);
            if (res != SUCCESS) {
                perror("aio_read() failed");
                return;
            }
            rqv[rqt++] = &readrq;
        } else {
            readrq.aio_nbytes = 0;
        }

        if (!empty && lines < MAXLINES) {
            char *t = buf + lowmark;
            char *l = (lowmark < highmark) ? buf + highmark : buf + sizeof(buf);

            for (; t < l && *t != '\n'; t++);
            if (t < l) {
                t++;
                lines++;
            }
            writerq.aio_buf = buf + lowmark;
            writerq.aio_nbytes = t - (buf + lowmark);
            res = aio_write(&writerq);
            if (res != SUCCESS) {
                perror("aio_write() failed");
                return;
            }
            rqv[rqt++] = &writerq;
        } else {
            writerq.aio_nbytes = 0;
        }

        if (lines >= MAXLINES) {
            termreadrq.aio_nbytes = 1;
            res = aio_read(&termreadrq);
            if (res != SUCCESS) {
                perror("aio_read() failed");
                return;
            }
            rqv[rqt++] = &termreadrq;
        } else {
            termreadrq.aio_nbytes = 0;
        }

        aio_suspend((const struct aiocb *const *)rqv, rqt, NULL);

        res = aio_error(&readrq);
        if (readrq.aio_nbytes && res != EINPROGRESS) {
            if (res != SUCCESS) {
                fprintf(stderr, "reading from socket: %s\n", strerror(res));
                return;
            }
            res = aio_return(&readrq);
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
        }

        res = aio_error(&writerq);
        if (writerq.aio_nbytes && res != EINPROGRESS) {
            if (res != SUCCESS) {
                fprintf(stderr, "writing to terminal: %s\n", strerror(res));
                return;
            }
            res = aio_return(&writerq);
            if (res == FAIL) {
                perror("aio_return() failed");
            }
            lowmark += res;
            if (lowmark == sizeof(buf)) {
                lowmark = 0;
            }
            if (lowmark == highmark) {
                empty = true;
            }
            full = false;
        }
        res = aio_error(&termreadrq);
        if (termreadrq.aio_nbytes && res != EINPROGRESS) {
            if (res != SUCCESS) {
                fprintf(stderr, "reading from terminal: %s\n", strerror(res));
                return;
            }
            lines = 0;
        }

        if (eof && empty) {
            break;
        }
    }
}
