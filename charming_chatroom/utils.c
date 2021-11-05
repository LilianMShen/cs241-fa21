// lmshen2, dbargon2, jlwang5, justink6

/**
 * charming_chatroom
 * CS 241 - Fall 2021
 */
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    int32_t s = htonl((uint32_t)size);
    ssize_t ret = write(socket, &s, sizeof(s));
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

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t ret = write_all_to_socket(socket, buffer, count);
    if (ret < 0) {
        perror(NULL);
        exit(1);
    }
    return ret;
}