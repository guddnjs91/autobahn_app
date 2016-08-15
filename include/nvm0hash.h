#ifndef nvm0hash_h
#define nvm0hash_h

#include "nvm0inode.h"
#include "nvm0list.h"
#include "libcuckoo/src/cuckoohash_map.hh"
#include "libcuckoo/src/city_hasher.hh"

struct hash_node
{
    struct inode_entry* inode;
    uint32_t            lbn;
    bool                is_valid;
    pthread_mutex_t     mutex;

    struct hash_node*   prev;
    struct hash_node*   next;
};

struct hash_table
{
    cuckoohash_map<uint32_t, struct hash_node*> map; //key = lbn
    struct hash_node_list* invalid_list;
};

#endif
