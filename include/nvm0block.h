#pragma once

#include <cstdint>

#ifndef BLOCK_SIZE
#define BLOCK_SIZE (1 << 14)
#endif

typedef uint32_t block_idx_t;

/* Represent one block object */
struct block_entry {
    char data[BLOCK_SIZE];
};

