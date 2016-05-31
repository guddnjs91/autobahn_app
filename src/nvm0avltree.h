#include "nvm0inode.h"

/**
 * AVL tree node.
 * Tree structure doesn't embedded in inode anymore.
 * T_node will be managed by AVL tree in MEMORY
 * Each T_node contins inode and its valid bit. */
typedef struct _tree_node {
   struct _nvm_inode* inode;
   int valid; // Balloon thread change this and writer will delegate this node.

   struct _tree_node* left;
   struct _tree_node* right;
   int height;
} T_node;
