/**
 * malloc
 * CS 241 - Fall 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _meta_data {
    // Number of bytes of heap memory the user requested from malloc
    size_t request_size;

    // Pointer to the next instance of meta_data in the list
    struct _meta_data *next;

    // Pointer to previous instance of meta_data in the list
    struct _meta_data *prev;
} meta_data;

static meta_data * dataHead;
static meta_data * freeHead;

meta_data * findFreeBlock(size_t size) {
    meta_data * block = freeHead;

    while (block != NULL) {
        if (size <= block->request_size) {
            return block;
        }
        block = block->next;
    }

    return NULL;
}

meta_data * findDataBlock(void * ptr) {
    void * findptr = ptr - 1;

    meta_data * block = dataHead;
    while (block != NULL) {
        if (block == findptr) return block; 
        block = block->next;
    }

    return NULL;
}

void removeFromFree(meta_data * block) {
    if (freeHead == block) {
        freeHead = block->next;
        if (block->next) block->next->prev = NULL;
    } else {
        if (block->prev) block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void removeFromData(meta_data * block) {
    if (dataHead == block) {
        dataHead = block->next;
        if (block->next) block->next->prev = NULL;
    } else {
        if (block->prev) block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void addToFree(meta_data * block) {
    if (freeHead == NULL) {
        freeHead = block;
        block->prev = NULL;
        block->next = NULL;
    } else {
        meta_data * prevBlock = freeHead;
        while (prevBlock -> next != NULL) {
            prevBlock = prevBlock->next;
        }
        prevBlock->next = block;
        block->prev = prevBlock;
    }
}

void addToData(meta_data * block) {
    if (dataHead == NULL) {
        dataHead = block;
        block->prev = NULL;
        block->next = NULL;
    } else { // add to the linked list
        meta_data * prevBlock = dataHead;
        while (prevBlock -> next != NULL) {
            prevBlock = prevBlock->next;
        }
        prevBlock->next = block;
        block->prev = prevBlock;
    }
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    size_t s = num * size;

    if (s == 0) return NULL;

    meta_data * block;

    if (freeHead != NULL) {
        block = findFreeBlock(s);

        if (block != NULL) {
            removeFromFree(block);
            addToData(block);

            // initialize all bytes to 0
            memset(block + 1, 0, s);

            return block + 1;
        }
    }

    block = sbrk(s + sizeof(meta_data));

    block->request_size = s;

    addToData(block);

    // initialize all bytes to 0
    memset(block + 1, 0, s);

    return block + 1;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    if (size == 0) return NULL;

    meta_data * block;

    if (freeHead != NULL) {
        block = findFreeBlock(size);

        if (block != NULL) {
            removeFromFree(block);
            addToData(block);

            return block + 1;
        }
    }
    
    // block = sbrk(0);
    block = sbrk(0);
    if (sbrk(sizeof(meta_data) + size) == (void*) - 1) return NULL;
    // block = sbrk(size + sizeof(meta_data));

    block->request_size = size;

    addToData(block);

    return block + 1;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if (ptr == NULL) {
        return;
    }

    meta_data * block = ptr - sizeof(meta_data);

    if (block == NULL || dataHead == NULL) return;

    removeFromData(block);

    addToFree(block);
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (ptr == NULL) {
        return malloc(size);
    }

    meta_data * blockIterator = findDataBlock(ptr);

    if (size == 0) {
        if (blockIterator != NULL) free(ptr);
        return NULL;
    }

    if (blockIterator != NULL) {
        if (size > blockIterator->request_size) {
            void * returnPtr = malloc(size);
            if (returnPtr == NULL) {
                return NULL;
            }

            memcpy(returnPtr, ptr, blockIterator->request_size);
            free(ptr);
        } else {
            return ptr;
        }
    }

    void * returnPtr = malloc(size);
    if (returnPtr == NULL) {
        return NULL;
    }

    memcpy(returnPtr, ptr, blockIterator->request_size);
    free(ptr);
    return returnPtr;
}
