#ifndef nvm0hash_h
#define nvm0hash_h

#include "nvm0inode.h"
#include "nvm0list.h"
#include <unordered_map>

struct hash_node
{
    struct inode_entry* inode;
    uint32_t            lbn;
    bool                is_valid;
    struct hash_node*   prev;
    struct hash_node*   next;
};

struct hash_table
{
    std::unordered_map<uint32_t, struct hash_node*> map; //key = lbn
    struct list* invalid_list;
};

#endif
