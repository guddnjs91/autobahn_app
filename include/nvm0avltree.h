/**
 * AVL tree node.
 * Tree structure doesn't embedded in inode anymore.
 * tree_node will be managed by AVL tree in MEMORY
 * Each tree_node contins inode and its valid bit. */

#ifndef nvm0avltree_h
#define nvm0avltree_h

#include "nvm0inode.h"

#define TREE_VALID      0
#define TREE_INVALID    1

struct tree_root
{
    struct tree_node *root;
    int count_total;
    int count_invalid;
};

struct tree_node 
{
   struct inode_entry*    inode;
   uint32_t               lbn; // key
   int                    valid; // Balloon thread change this and writer will delegate this node.

   struct tree_node*      left;
   struct tree_node*      right;
   int                    height;
};

#endif
