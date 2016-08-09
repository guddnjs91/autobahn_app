#ifndef nvm0hash_h
#define nvm0hash_h

#include "nvm0inode.h"
#include "nvm0list.h"
#include <unordered_map>

#define HASH_NODE_VALID 0
#define HASH_NODE_INVALID 1

struct hash_node
{
    struct inode_entry* inode;
    int                 valid;

    struct hash_node*   prev;
    struct hash_node*   next;
};

struct hash_table
{
    std::unordered_map<uint32_t, struct hash_node*> map;
    int count_total;
    int count_invalid;
    struct list* invalid_list;
};

#endif
