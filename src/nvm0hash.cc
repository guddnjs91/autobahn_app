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
    h->lbn = inode->lbn;
    h->valid = HASH_NODE_VALID;

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
    hash_node *searched_node = search_hash_node(table, node->lbn);
    if (searched_node && searched_node->valid == HASH_NODE_VALID) 
    {
        searched_node->valid = HASH_NODE_INVALID;
        table->count_invalid++;
    }
}

/**
Delete hash node from hash table. */
void
physical_delete_hash_node(
    hash_table* table,
    hash_node* node)
{
    table->map.erase(node->lbn);
    table->count_total--;
}

//int main ()
//{
//  std::unordered_map<std::string,std::string> mymap;
//  mymap = {{"Australia","Canberra"},{"U.S.","Washington"},{"France","Paris"}};
//
//  std::cout << "mymap contains:";
//  for ( auto it = mymap.begin(); it != mymap.end(); ++it )
//    std::cout << " " << it->first << ":" << it->second;
//  std::cout << std::endl;
//
//  std::cout << "mymap's buckets contain:\n";
//  for ( unsigned i = 0; i < mymap.bucket_count(); ++i) {
//    std::cout << "bucket #" << i << " contains:";
//    for ( auto local_it = mymap.begin(i); local_it!= mymap.end(i); ++local_it )
//      std::cout << " " << local_it->first << ":" << local_it->second;
//    std::cout << std::endl;
//  }
//
//  return 0;
//}
