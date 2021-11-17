// partners: lmshen2, justink6, dbargon2, jlwang5

/**
 * mad_mad_access_patterns
 * CS 241 - Fall 2021
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "utils.h"
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

BinaryTreeNode* read_node(FILE* file, long abs_offset) {
  // Seek file to start of word
  fseek(file, abs_offset + sizeof(BinaryTreeNode), SEEK_SET);
  long word_length;
  for (word_length = 0; fgetc(file) != 0; word_length++);

  // Seek back to beginning of node
  fseek(file, abs_offset, SEEK_SET);
  // Plus one for null byte
  BinaryTreeNode* ret = malloc(sizeof(BinaryTreeNode) + word_length + 1);

  if (fread(ret, 1, sizeof(BinaryTreeNode) + word_length + 1, file) != sizeof(BinaryTreeNode) + word_length + 1) {
    free(ret);
    return NULL;
  }
  return ret;

}

// Finds BinaryTreeNode given a word, destroys all nodes except return variable
BinaryTreeNode* find_node(FILE* file, BinaryTreeNode* root, char* word) {
  int comp = strcmp(word, root->word);
  if (comp == 0) {
    return root;
  } else if (comp < 0) {
    if (root->left_child == 0) {
      free(root);
      return NULL;
    } 
    BinaryTreeNode* next = read_node(file, root->left_child);
    free(root);
    return find_node(file, next, word);
  } else {
    if (root->right_child == 0) {
      free(root);
      return NULL;
    }
    BinaryTreeNode* next = read_node(file, root->right_child);
    free(root);
    return find_node(file, next, word);
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printArgumentUsage();
    return -1;
  }

  // argv[1] = data_file
  // argv[2+] = keywords
  char * fileName = argv[1];

  FILE * input = fopen(fileName, "r");
  if (input == NULL){
    openFail(fileName);
    return -1;
  }

  // The first 4 bytes of the input file are the letters “BTRE”. 
  // Any file which does not start with these 4 bytes is an invalid data file

  char firstFour[4];
  fread(firstFour, 1, 4, input);
  if (strcmp(firstFour, BINTREE_HEADER_STRING) != 0) {
    formatFail(fileName);
    return -1;
  }

  // Get root node
  int i;
  for (i = 2; i < argc; i++) {
    BinaryTreeNode *root = read_node(input, BINTREE_ROOT_NODE_OFFSET);
    BinaryTreeNode *target = find_node(input, root, argv[i]);
    if (target == NULL) {
      printNotFound(argv[i]);
    } else {
      printFound(argv[i], target->count, target->price);
      free(target);
    }
  }
  fclose(input);
  
  return 0;
}