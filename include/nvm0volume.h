#pragma once

#include <cstdint>

typedef uint32_t volume_idx_t;

/* Represent one volume table entry */
struct volume_entry {
    uint32_t            vid;    // volume id - implicit filename
    int                 fd;     // file descriptor
    struct hash_table*  hash_table;   
};

/* functions */
volume_idx_t get_volume_entry_idx(uint32_t vid);
volume_idx_t search_volume_entry_idx(uint32_t vid);
volume_idx_t alloc_volume_entry_idx(uint32_t vid);
