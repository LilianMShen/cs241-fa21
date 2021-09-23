/**
 * mini_memcheck
 * CS 241 - Fall 2021
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

extern meta_data *head = NULL;
extern size_t total_memory_requested = 0;
extern size_t total_memory_freed = 0;
extern size_t invalid_addresses = 0;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    if (request_size == 0) {
        return NULL;
    }

    meta_data * block = malloc(sizeof(meta_data) + request_size);

    // if allocation fails
    if (block == NULL) {
        printf("allocation failed");
        return NULL;
    }

    total_memory_requested += request_size;

    block->request_size = request_size;
    block->filename = filename;
    block->instruction = instruction;
    block->next = NULL;
    // if no head, then head = block;
    if (head == NULL) {
        printf("made head\n");
        head = block;
    } else { // add to the linked list
        // printf("made head\n");
        meta_data * nextBlock = head;
        while (nextBlock -> next != NULL) {
            nextBlock = nextBlock->next;
        }
        nextBlock -> next = block;
    }
    return (block + sizeof(meta_data));
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if (num_elements == 0 || element_size == 0) {
        return NULL;
    }

    meta_data * nextBlock = head->next;
    while (nextBlock != NULL) {
        nextBlock = nextBlock->next;
    }

    size_t request_size = num_elements * element_size;

    nextBlock = malloc(sizeof(meta_data) + request_size);

    if (nextBlock == NULL) {
        return NULL;
    }

    // initialize all data to 0;
    memset(nextBlock, 0, sizeof(meta_data) + request_size);

    total_memory_requested += request_size;

    nextBlock->request_size = request_size;
    nextBlock->filename = filename;
    nextBlock->instruction = instruction;
    nextBlock->next = NULL;
    return (nextBlock + sizeof(meta_data));
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (payload == NULL) {
        return mini_malloc(request_size, filename, instruction);
    }

    if (request_size == 0) {
        mini_free(payload);
        return NULL;
    }

    // FIND THE BLOCK THAT CONTAINS THE POINTER
    meta_data * block = head;
    void * findPtr;
    meta_data * prevBlock;

    while (block != NULL) {
        findPtr = block + sizeof(meta_data);
        if (findPtr == payload) {
            printf("ptr found for realloc\n");
            break;
        }
        findPtr = NULL;
        prevBlock = block;
        block = block->next;
    }

    // couldnt find the pointer in the allocated memoryyyy
    if (findPtr == NULL || block == NULL) {
        printf("realloc invalid address hit\n");
        invalid_addresses += 1;
        return NULL;
    }

    if (block->request_size < request_size) {
        total_memory_requested += (request_size - block->request_size);
    } else if (block->request_size > request_size) {
        total_memory_freed += (request_size - block->request_size);
    }

    meta_data * nBlock = block->next;
    
    // do i realloc the block??? or the payload pointer??? OR
    block = realloc(block, sizeof(meta_data) + request_size);

    // failed to reallocate
    if (block == NULL) {
        return NULL;
    }

    if (prevBlock == NULL) {
        head = block;
    } else {
        prevBlock->next = block;
    }

    block->request_size = request_size;
    block->filename = filename;
    block->instruction = instruction;
    block->next = nBlock;

    printf("successful reallocation\n");
    return (block + sizeof(meta_data));
}

void mini_free(void *payload) {
    // your code here

    if (payload == NULL) {
        return;
    }

    // FIND THE BLOCK THAT CONTAINS THE POINTER
    meta_data * block = head;
    void * findPtr = (block + sizeof(meta_data));
    meta_data * prevBlock;
    
    // if (findPtr == payload) printf("hit");
    
    while (block != NULL) {
        printf("cycle hit \n");
        findPtr = (block + sizeof(meta_data));
        if (findPtr == payload) {
            printf("hit\n");
            break;
        }
        findPtr = NULL;
        prevBlock = block;
        block = block->next;
    }

    printf("is block null? %d\n", (block == NULL));

    // couldnt find the pointer in the allocated memoryyyy
    if (findPtr == NULL) {
        printf("invalid address hit\n");
        invalid_addresses += 1;
        return;
    }

    if (block == head) {
        head = block->next;
        total_memory_freed += block->request_size;
        free(block);
    } else {
        prevBlock->next = block->next;
        total_memory_freed += block->request_size;
        free(block);
    }
}
