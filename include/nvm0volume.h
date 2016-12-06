#pragma once

#include <cstdint>
#include "nvm0block.h"

typedef uint32_t volume_idx_t;

/* Represent one volume table entry */
struct volume_entry {
    uint32_t            vid;            // volume id - implicit filename
    int                 fd;             // file descriptor
    block_idx_t         block_index;    // block position index
    struct hash_table*  hash_table;
};


/* functions */
volume_entry *get_volume_entry(volume_idx_t v_idx);
volume_idx_t get_volume_entry_idx(uint32_t vid);
volume_idx_t alloc_volume_entry_idx(uint32_t vid);

const char *get_filename(uint32_t vid);
off_t get_filesize(uint32_t vid);
