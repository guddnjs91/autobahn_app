#pragma once

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
    cuckoohash_map<uint32_t, struct hash_node*> map;
    struct hash_node_list* invalid_list;
};

/* functions */
struct hash_table* new_hash_table();
void validate_hash_node(struct hash_node* hash_node, struct inode_entry* inode);
struct hash_node* new_hash_node(inode_entry* inode);
void insert_hash_node(struct hash_table *table, hash_node *node);
struct hash_node* search_hash_node(struct hash_table *table, uint32_t lbn);
void logical_delete_hash_node(struct hash_table *table, hash_node *node);
void physical_delete_hash_node(struct hash_table *table, hash_node *node);
