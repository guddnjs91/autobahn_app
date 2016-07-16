/**
 * AVL tree node.
 * Tree structure doesn't embedded in inode anymore.
 * T_node will be managed by AVL tree in MEMORY
 * Each T_node contins inode and its valid bit. */

#ifndef nvm0avltree_h
#define nvm0avltree_h

struct tree_node {
   struct inode_entry*  inode;
   int                  valid; // Balloon thread change this and writer will delegate this node.

   struct tree_node*    left;
   struct tree_node*    right;
   int                  height;
};

tree_node* search_nvm_inode(tree_node* root, uint32_t lbn);
tree_node* insert_nvm_inode(tree_node* root, tree_node* inode);
tree_node* delete_nvm_inode(tree_node* root, tree_node* inode);
void deallocate_node(tree_node* root);
tree_node* min_value_node(tree_node* inode);
int Max(int a, int b);
int height(tree_node* N);
int getBalance(tree_node* N);
tree_node* rightRotate(tree_node* y);
tree_node* leftRotate(tree_node* y);

#endif
