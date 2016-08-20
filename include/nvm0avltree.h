#pragma once

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
   uint32_t               lbn;
   int                    valid;

   struct tree_node*      left;
   struct tree_node*      right;
   int                    height;
};



/* functions */
tree_node* search_tree_node(tree_root* tree, uint32_t lbn);
void insert_tree_node(tree_root* tree, tree_node* node);
tree_node* insert_tree_node(tree_node* root, tree_node* node);
tree_node* physical_delete_tree_node(tree_node* root, tree_node* node);
void logical_delete_tree_node(tree_root* tree, tree_node* node);
void rebalance_tree_node(tree_root* tree);
tree_node* find_invalid_tree_node(tree_node* node);

tree_node* init_tree_node(inode_entry* inode);
tree_node* min_value_node(tree_node* node);
int max_height(int a, int b);
int get_height(tree_node* node);
int get_balance(tree_node* node);
tree_node* right_rotate(tree_node* y);
tree_node* left_rotate(tree_node* x);
double get_invalid_ratio(tree_root *tree);
