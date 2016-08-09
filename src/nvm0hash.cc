#include <stdlib.h>
#include <unordered_map>
#include "nvm0common.h"

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
    remove_list_node(inode->volume->hash_table->invalid_list, hash_node);
}

/**
 * Insert a hash node to a hash table.
 */
void
insert_hash_node(
    struct hash_table* table,
    struct hash_node* node)
{
    std::pair<uint32_t, hash_node*> pair (node->lbn, node);
    table->map.insert(pair);
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
    std::unordered_map<uint32_t, hash_node*>::iterator it = table->map.find(lbn);

    if(it == table->map.end()) {
        return nullptr;
    }

    if(it->second == nullptr) {
        printf("ERROR: hash has a mapped value nullptr!\n");
    }
    return it->second;
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
