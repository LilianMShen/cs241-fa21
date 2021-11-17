// partners: lmshen2, justink6, dbargon2, jlwang5

/**
 * Mad Mad Access Pattern
 * CS 241 - Fall 2016
 */
#include "tree.h"
#include "utils.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.
  ./lookup2 <data_file> <word> [<word> ...]
*/

static void *data;

void search_for_word(int offset, char *word) {
    if (offset <= 0) {
        printNotFound(word);
        return;
    }

    BinaryTreeNode *node = (BinaryTreeNode *) (data + offset);

    int check = strcmp(word, node->word);
    if (check == 0) {
        printFound(word, node->count, node->price);
        return;
    } else if (check > 0) {
        search_for_word(node->right_child, word);
    } else {
        search_for_word(node->left_child, word);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printArgumentUsage();
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    struct stat s;

    if (fd < 0 || fstat(fd, &s) < 0) {
        openFail(argv[1]);
        exit(2);
    } else {
      // check for invalid data file
      char buf[5];
      read(fd, buf, 4);
      buf[4] = '\0';
      if (strcmp(buf, "BTRE") != 0) {
        formatFail(argv[1]);
        exit(2);
      }
    }

    data = mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        mmapFail(argv[1]);
        exit(3);
    }

    close(fd);

    for (int i = 2; i < argc; i++) {
        search_for_word(BINTREE_ROOT_NODE_OFFSET, argv[i]);
    }

    exit(0);
}