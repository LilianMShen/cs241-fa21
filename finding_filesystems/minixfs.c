/**
 * finding_filesystems
 * CS 241 - Fall 2021
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void * get_data_block(file_system * fs, inode * node, size_t index) {
    data_block_number * blockNum;
    if (index < NUM_DIRECT_BLOCKS) {
        blockNum = node->direct;
    } else {
        blockNum = (data_block_number *) (fs->data_root + node->indirect);
        index -= NUM_DIRECT_BLOCKS;
    }
    return (fs->data_root + blockNum[index]);
}

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

/**
 *  chmod takes the inode at path and a set of permissions
 *  and sets the permissions.
 *
 *  Note that you should only overwrite the bottom RWX_BITS_NUMBER bytes.
 *  The top 7 bits are reserved for the type of the file.
 *
 *  This function should update the node's ctim
 *
 *  Return value:
 *    On success return 0
 *    On failure return -1
 *      If this functions fails because path doesn't exist,
 *      set errno to ENOENT
 *
 */
int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    inode * node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        return -1;
    }

    //Note that you should only overwrite the bottom RWX_BITS_NUMBER bytes. The top 7 bits are reserved for the type of the file.
    // bit shift?
    uint16_t left = node->mode>>RWX_BITS_NUMBER;
    // bit mask?
    node->mode = new_permissions | (left << RWX_BITS_NUMBER);

    clock_gettime(CLOCK_REALTIME, &node->ctim);

    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode * node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        return -1;
    }

    if (owner != ((uid_t)-1)) node->uid = owner;
    if (group != ((gid_t)-1)) node->gid = group;

    clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    if (valid_filename(path) != 1) return NULL;

    inode * node = get_inode(fs, path);

    if (node != NULL) {
        clock_gettime(CLOCK_REALTIME, &node->ctim);
        return NULL;
    }

    char * fname = NULL;
    inode * parent = parent_directory(fs, path, (const char**) &fname);
    int isDirectory = is_directory(parent);
    if (parent == NULL || isDirectory != 1) return NULL;

    inode_number first = first_unused_inode(fs);
    if (first == -1) return NULL;

    inode * new = fs->inode_root + first;
    init_inode(parent, new);
    minixfs_dirent dirent;
    dirent.name = fname;
    dirent.inode_num = first;

    size_t index = parent->size / sizeof(data_block);
    if (index >= NUM_DIRECT_BLOCKS) return NULL;
    size_t offset = parent->size % sizeof(data_block);

    if (add_data_block_to_inode(fs, parent) == -1) return NULL;

    void * blockaddr = get_data_block(fs, parent, index);
    blockaddr += offset;
    memset(blockaddr, 0, sizeof(data_block));
    make_string_from_dirent(blockaddr, dirent);

    parent->size += MAX_DIR_NAME_LEN;

    clock_gettime(CLOCK_REALTIME, &parent->mtim);
    return NULL;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        ssize_t visited = 0;
        char * map = GET_DATA_MAP(fs->meta);
        for (uint64_t i = 0; i < fs->meta->dblock_count; i++) {
            if (map[i] == 1) visited++;
        }

        char * info_string = block_info_string(visited);
        size_t string_len = strlen(info_string);
        if ((size_t) *off > string_len) return 0;
        if (count > string_len - *off) count = string_len - *off;
        memmove(buf, info_string + *off, count);
        *off += count;
        return count;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    size_t max = sizeof(data_block) * (NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS);
    if (count + *off > max) {
        errno = ENOSPC;
        return -1;
    }

    int block_count = fs->meta->dblock_count;
    int min = minixfs_min_blockcount(fs, path, block_count);
    if (min == -1) {
        errno = ENOSPC;
        return -1;
    }

    inode * node = get_inode(fs, path);
    if (node == NULL) {
        node = minixfs_create_inode_for_path(fs, path);
        if (node == NULL) {
            errno = ENOSPC;
            return -1;
        }
    }

    
    data_block_number dbn = add_data_block_to_inode(fs, node);
    if (dbn == -1) {
        errno = ENOSPC;
        return -1;
    }
    // inode_number i = add_single_indirect_block(fs, node);
    // if (i == -1) {
    //     errno = ENOSPC;
    //     return -1;
    // }
    // data_block_number addDB = add_data_block_to_indirect_block(fs, &node->indirect);
    // if (addDB == -1) {
    //     errno = ENOSPC;
    //     return -1;
    // }

    size_t index = *off / sizeof(data_block);
    size_t offset = *off % sizeof(data_block);
    size_t size = count;
    if (count > sizeof(data_block) - offset) size = sizeof(data_block) - offset;
    size_t written = size;
    *off += size;
    void * blockaddr = get_data_block(fs, node, index);
    blockaddr += offset;
    index++;
    memcpy(blockaddr, buf, size);
    while (written < count) {
        // if (count - written > sizeof(data_block)) size = sizeof(data_block);
        // else size = count - written;
        blockaddr = get_data_block(fs, node, index);
        memcpy(blockaddr, buf + written, size);
        written += size;
        *off += size;
        index++;
    }

    if (count + *off > node->size) node->size = count + *off;

    clock_gettime(CLOCK_REALTIME, &node->atim);
    clock_gettime(CLOCK_REALTIME, &node->mtim);


    return written;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!

    inode * node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        return -1;
    }
    /*If *off is greater than the end of the file,
 *  do nothing and return 0 to indicate end of file. */
    if ((uint64_t) *off > node->size) return 0;
    
    if (node->size - *off < count) {
        count = node->size - *off;
    }

    size_t index = *off / sizeof(data_block);
    size_t offset = *off % sizeof(data_block);
    size_t size = count;
    if (count > sizeof(data_block) - offset) size = sizeof(data_block) - offset;
    size_t read = size;
    *off += size;
    void * blockaddr = get_data_block(fs, node, index);
    blockaddr += offset;
    index++;
   while (read < count) {
    //    if (count - read > sizeof(data_block)) size = sizeof(data_block);
    //    else size = count - read;

       blockaddr = get_data_block(fs, node, index);
       memcpy(buf + read, blockaddr, size);
       read += size;
       *off += size;
       index++; 
   }

   clock_gettime(CLOCK_REALTIME, &node->atim);
    return read;
}
