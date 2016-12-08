#pragma once

#include "nvm0inode.h"
#include "libcuckoo/src/cuckoohash_map.hh"
#include "libcuckoo/src/city_hasher.hh"

struct hash_node
{
    struct inode_entry* inode;
    uint32_t            lbn;
    bool                is_valid;
    pthread_mutex_t     mutex;
};

struct hash_table
{
    cuckoohash_map<uint32_t, struct hash_node*> map;
};

/* functions */
struct hash_table* new_hash_table();
void validate_hash_node(struct hash_node* hash_node, struct inode_entry* inode);
struct hash_node* new_hash_node(inode_entry* inode);
void insert_hash_node(struct hash_table *table, hash_node *node);
struct hash_node* search_hash_node(struct hash_table *table, uint32_t lbn);
void logical_delete_hash_node(struct hash_table *table, hash_node *node);
void physical_delete_hash_node(struct hash_table *table, hash_node *node);
