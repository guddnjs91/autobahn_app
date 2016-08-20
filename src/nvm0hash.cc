#include "nvm0hash.h"

/**
 * Constructor for hash_table structure.
 */
struct hash_table*
new_hash_table()
{
    struct hash_table* hash_table = new struct hash_table;
    hash_table->invalid_list = new_list();

    return hash_table;
}

/**
 * Allocate and initailize a new hash node with given inode.
 * @return initialized hash node
 */
struct hash_node*
new_hash_node(
    struct inode_entry* inode ) /*!< in: embedding inode */
{
    struct hash_node* node = new struct hash_node;
    node->inode = inode;
    node->lbn = inode->lbn;
    node->is_valid = true;
    node->mutex = PTHREAD_MUTEX_INITIALIZER;
    node->prev = nullptr;
    node->next = nullptr;

    return node;
}

void
validate_hash_node(
    struct hash_node* hash_node,
    struct inode_entry* inode )
{
    hash_node->inode = inode;
    hash_node->lbn = inode->lbn;
    hash_node->is_valid = true;
}

/**
 * Insert a hash node to a hash table.
 */
void
insert_hash_node(
    struct hash_table* table,
    struct hash_node* node)
{
    table->map[node->inode->lbn] = node;
}

/**
 * Search the hash table for a hash node.
 * @return a found hash node, or nullptr if not found
 */
struct hash_node*
search_hash_node(
    struct hash_table* table,
    uint32_t lbn)
{
    struct hash_node *node = nullptr;
    table->map.find(lbn, node);

    return node;
}

/**
 * Logically deletes a hash node and inserts it into an invalid_list.
 */
void
logical_delete_hash_node(
    struct hash_table* table,
    struct hash_node* node)
{
    node->inode = nullptr;
    node->is_valid = false;
    push_back_list_node(table->invalid_list, node);
}

/**
 * Physically deletes a hash node from hash table.
 */
void
physical_delete_hash_node(
    struct hash_table* table,
    uint32_t lbn)
{
    table->map.erase(lbn);
}
