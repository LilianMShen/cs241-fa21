/**
 * nonstop_networking
 * CS 241 - Fall 2021
 */
#include "common.h"
#include "format.h"
#include <sys/epoll.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/stat.h>

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t ret = write(socket, buffer, count);
    if (ret < 0) {
        perror(NULL);
        exit(1);
    }
    return ret;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    char *pbuf = buffer;
    size_t buflen = count;
    while (buflen > 0) {
        int bytes_read = read(socket, pbuf, buflen); // Receive bytes

        if (bytes_read == -1 && errno == EINTR) {
            continue;
        }

        if (bytes_read <= 0) {
            return bytes_read;
        } 

        pbuf += bytes_read;
        buflen -= bytes_read;
    }

    return count;
}
