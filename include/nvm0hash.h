#ifndef nvm0hash_h
#define nvm0hash_h

#include "nvm0inode.h"
#include <unordered_map>

#define HASH_NODE_VALID 0
#define HASH_NODE_INVALID 1

struct hash_node
{
    struct inode_entry* inode;
    uint32_t            lbn;
    int                 valid;
};

struct hash_table
{
    std::unordered_map<uint32_t, struct hash_node*> map;
    int count_total;
    int count_invalid;
};

#endif
