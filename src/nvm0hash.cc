#include "nvm0hash.h"

/**
 * Constructor for hash_table structure.
 */
struct hash_table *new_hash_table()
{
    return new struct hash_table;
}

/**
 * Allocate and initailize a new hash node with given inode.
 * @return initialized hash node
 */
struct hash_node *new_hash_node(inode_entry* inode)
{
    struct hash_node *hash_node = new struct hash_node;

    hash_node->inode    = inode;
    hash_node->lbn      = inode->lbn;
    hash_node->is_valid = true;
    hash_node->mutex    = PTHREAD_MUTEX_INITIALIZER;

    return hash_node;
}

void validate_hash_node(
        struct hash_node    *hash_node,
        struct inode_entry  *inode)
{
    hash_node->inode    = inode;
    hash_node->lbn      = inode->lbn;
    hash_node->is_valid = true;
}

/**
 * Insert a hash node to a hash table.
 */
void insert_hash_node(
        struct hash_table *table,
        struct hash_node  *hash_node)
{
    table->map[hash_node->inode->lbn] = hash_node;
}

/**
 * Search the hash table for a hash node.
 * @return a found hash node, or nullptr if not found
 */
struct hash_node *search_hash_node(
        struct hash_table *table,
        uint32_t lbn)
{
    struct hash_node *hash_node = nullptr;
    table->map.find(lbn, hash_node);

    return hash_node;
}

/**
 * Logically deletes a hash node and inserts it into an invalid_list.
 */
void logical_delete_hash_node(
        struct hash_table *table,
        struct hash_node *hash_node)
{
    hash_node->inode = nullptr;
    hash_node->is_valid = false;
}

/**
 * Physically deletes a hash node from hash table.
 */
void physical_delete_hash_node(
        struct hash_table *table,
        uint32_t lbn)
{
    table->map.erase(lbn);
}
