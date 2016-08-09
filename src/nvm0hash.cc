<<<<<<< HEAD
#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include "nvm0common.h"


/**
Allocate and initailize a hash node with inode
@return initialized hash node */
hash_node*
init_hash_node(
    inode_entry* inode) /*!< in: embedding inode */
{
    hash_node* h = (hash_node*)malloc(sizeof(hash_node));
    h->inode = inode;
    h->valid = HASH_NODE_VALID;
    h->prev = nullptr;
    h->next = nullptr;

    return h;
}

/**
Insert hash node to hash table */
void
insert_hash_node(
    hash_table* table,
    hash_node* node)
{
    std::pair<uint32_t, hash_node*> p (node->inode->lbn, node);
    table->map.insert(p);
    table->count_total++;
}

/**
Search hash node from hash table.
@return found hash node or nullptr if not found */
hash_node*
search_hash_node(
    hash_table* table,
    uint32_t lbn)
{
    std::unordered_map<uint32_t, hash_node*>::iterator it;
    it = table->map.find(lbn);

    if(it == table->map.end())
    {
        return nullptr;
    }

    return it->second;
}

/**
Logically delete hash node. */
void
logical_delete_hash_node(
    hash_table* table,
    hash_node* node)
{
    node->valid = HASH_NODE_INVALID;
    table->count_invalid++;
}

/**
Delete hash node from hash table. */
void
physical_delete_hash_node(
    hash_table* table,
    hash_node* node)
{
    table->map.erase(node->inode->lbn);
    table->count_total--;
}

bool isValidNode(struct hash_node *node)
{
    return node && (node->valid == HASH_NODE_VALID) ? true : false;
}
