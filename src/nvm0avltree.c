#include <stdio.h>
#include "nvm0common.h"
#include "nvm0avltree.h"


extern NVM_metadata* NVM;

/**
 * Search tree node object from avl tree root
 @return tree node with inode's lbn from avl-tree */
T_node*
search_nvm_inode(
    T_node* root,
    unsigned int lbn)
{
    T_node* localRoot = root;

    while(localRoot != NULL) {
        if(lbn == localRoot->inode->lbn) {
            return localRoot;
        } else if(lbn < localRoot->inode->lbn) {
            localRoot = localRoot->left;
        } else if(lbn > localRoot->inode->lbn) {
            localRoot = localRoot->right;
        }
    }
    return NULL;
}

/**
 * Insert tree node object to avl tree
 @return root node of avl-tree*/
T_node*
insert_nvm_inode(
    T_node* root,
    T_node* node)
{
    if (root == NULL) {
        root = node;
        return root;
    }

    if (node->inode->lbn < root->inode->lbn) {
        root->left = insert_nvm_inode(root->left, node);
    } else {
        root->right = insert_nvm_inode(root->right, node);
    }

    //update height
    root->height = Max(height(root->left), height(root->right)) + 1;

    //rebalancing
    int balance = getBalance(root);
    
    // LL
    if (balance > 1 && node->inode->lbn < root->left->inode->lbn) {
        return rightRotate(root);
    }
    
    // LR
    if (balance > 1 && node->inode->lbn > root->left->inode->lbn) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }   
    
    // RR
    if (balance < -1 && node->inode->lbn > root->right->inode->lbn) {
        return leftRotate(root);
    }
    
    // RL
    if (balance < -1 && node->inode->lbn < root->right->inode->lbn) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

/**
 * Delete tree node object from avl tree 
 * @return tree after deleting the node */
T_node*
delete_nvm_inode(
    T_node* root,  /* !<out: sub-tree root after deleting node */
    T_node* node) /* !<in: node to be deleted */
{
    if (root == NULL) {
        return root;
    }
    
    if (node->inode->lbn < root->inode->lbn) {
        root->left = delete_nvm_inode(root->left, node);
    } else if (node->inode->lbn > root->inode->lbn) {
        root->right = delete_nvm_inode(root->right, node);
    } else {
        // 1 child || no child
        if ((root->left == NULL) || (root->right == NULL)) {
            T_node* temp = root->left ? root->left : root->right;
            
            if (temp == NULL) { // no child
                temp = root;
                root = NULL;
            } else  { // 1 child, copy the contents of the non-empty child
		*root = *temp;
            }

            deallocate_node(temp);
        } else {
            
            // 2 children
            T_node* temp =  min_value_node(root->right);
            
            // copy the inorder successor's data
            root->inode->lbn = temp->inode->lbn;
            
            // Delete the inorder successor
            root->right = delete_nvm_inode(root->right, temp);
	}
    }
    
    if (root == NULL) {
        return root;
    }
    
    //update height
    root->height = Max(height(root->left), height(root->right)) + 1;
    
    //rebalancing
    int balance = getBalance(root);
    
    //LL
    if (balance > 1 && getBalance(root->left) >= 0) {
        return rightRotate(root);
    }
    
    //LR
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    
    //RR
    if (balance < -1 && getBalance(root->right) <= 0) {
        return leftRotate(root); 
    }
    
    //RL
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    
    return root;
}

/**
 * Find the minimum key from AVL tree.
 * @return tree node that has minimum key value in tree.*/
T_node*
min_value_node(
    T_node* node)
{
    T_node* current = node;
    
    while (current->left != NULL) {
        current = current->left;
    }
    
    return current;
}

/**
 * Deallocate tree's inode structure */
void
deallocate_node(
    T_node* node)
{
    // Re-initialization
    node->inode->lbn = 0;
    node->inode->state = INODE_STATE_FREE;
    node->inode->vte = nullptr;
    node->height = 1;
    node->left = nullptr;
    node->right = nullptr;
}

/**
 * Get the height of avl-tree
 @return height */
int
height(
    T_node* node)
{
    if (node == NULL) {
        return 0;
    }
    return node->height;
}

int
Max(
    int a,
    int b)
{
    return a > b ? a : b;
}

int
getBalance(
    T_node* inode)
{
    if (inode == NULL) {
        return 0;
    }

    return height(inode->left) - height(inode->right);
}

T_node*
rightRotate(
    T_node* y)
{
    T_node* x = y->left;
    T_node* T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = Max(height(y->left), height(y->right)) + 1;
    x->height = Max(height(x->left), height(x->right)) + 1;

    // Return new root
    return x;
}

T_node*
leftRotate(
    T_node* x)
{
    T_node* y = x->right;
    T_node* T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    x->height = Max(height(x->left), height(x->right)) + 1;
    y->height = Max(height(y->left), height(y->right)) + 1;

    // Return new root
    return y;
}
