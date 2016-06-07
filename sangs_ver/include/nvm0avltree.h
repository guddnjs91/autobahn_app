/**
 * AVL tree node.
 * Tree structure doesn't embedded in inode anymore.
 * T_node will be managed by AVL tree in MEMORY
 * Each T_node contins inode and its valid bit. */

#ifndef nvm0avltree_h
#define nvm0avltree_h

#include "nvm0inode.h"

typedef struct tree_node {
   struct inode_entry*  inode;
   int                  valid; // Balloon thread change this and writer will delegate this node.

   struct tree_node*    left;
   struct tree_node*    right;
   int                  height;
} T_node;

T_node* search_nvm_inode(T_node* root, unsigned int lbn);
T_node* insert_nvm_inode(T_node* root, T_node* inode);
T_node* delete_nvm_inode(T_node* root, T_node* inode);
void deallocate_node(T_node* root);
T_node* min_value_node(T_node* inode);
int Max(int a, int b);
int height(T_node* N);
int getBalance(T_node* N);
T_node* rightRotate(T_node* y);
T_node* leftRotate(T_node* y);

#endif
