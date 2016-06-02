#ifndef _NVM_AVL_TREE_H_
#define _NVM_AVL_TREE_H_

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
