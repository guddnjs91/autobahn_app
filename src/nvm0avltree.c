#include <stdio.h>
#include "nvm0common.h"

extern NVM_metadata* NVM;

void deallocateNode(NVM_inode* root);
NVM_inode* minValueNode(NVM_inode* inode);

/**
 * Search NVM_inode object from avl tree root
 @return inode with lbn from avl-tree */
NVM_inode*
search_nvm_inode(
    NVM_inode* root,
    unsigned int lbn)
{
    NVM_inode* localRoot = root;

    while(localRoot != NULL) {
        if(lbn == localRoot->lbn) {
            return localRoot;
        } else if(lbn < localRoot->lbn) {
            localRoot = localRoot->left;
        } else if(lbn > localRoot->lbn) {
            localRoot = localRoot->right;
        }
    }
    return NULL;
}

/**
 * Insert NVM_inode object to avl tree
 @return root node of avl-tree*/
NVM_inode*
insert_nvm_inode(
    NVM_inode* root,
    NVM_inode* inode)
{
    if (root == NULL) {
        root = inode;
        return root;
    }

    if (inode->lbn < root->lbn) {
        root->left = insert_nvm_inode(root->left, inode);
    } else {
        root->right = insert_nvm_inode(root->right, inode);
    }

    //update height
    root->height = Max(height(root->left), height(root->right)) + 1;

    //rebalancing
    int balance = getBalance(root);
    
    // LL
    if (balance > 1 && inode->lbn < root->left->lbn) {
        return rightRotate(root);
    }
    
    // LR
    if (balance > 1 && inode->lbn > root->left->lbn) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }   
    
    // RR
    if (balance < -1 && inode->lbn > root->right->lbn) {
        return leftRotate(root);
    }
    
    // RL
    if (balance < -1 && inode->lbn < root->right->lbn) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

/**
 * Delete NVM_inode object from avl tree 
 * @return tree after deleting the inode */
NVM_inode*
delete_nvm_inode(
    NVM_inode* root,  /* !<out: sub-tree root after deleting inode */
    NVM_inode* inode) /* !<in: inode to be deleted */
{
    if (root == NULL) {
        return root;
    }
    
    if (inode->lbn < root->lbn) {
        root->left = delete_nvm_inode(root->left, inode);
    } else if (inode->lbn > root->lbn) {
        root->right = delete_nvm_inode(root->right, inode);
    } else {
        // 1 child || no child
        if ((root->left == NULL) || (root->right == NULL)) {
            NVM_inode* temp = root->left ? root->left : root->right;
            
            if (temp == NULL) { // no child
                temp = root;
                root = NULL;
            } else  { // 1 child, copy the contents of the non-empty child
		*root = *temp;
            }

            deallocateNode(temp);
        } else {
            
            // 2 children
            NVM_inode* temp =  minValueNode(root->right);
            
            // copy the inorder successor's data
            root->lbn = temp->lbn;
            
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


NVM_inode* minValueNode(NVM_inode* inode)
{
	NVM_inode* current = inode;

	while (current->left != NULL)
		current = current->left;

	return current;
}

void deallocateNode(NVM_inode* root)
{
	// Re-initialization
	root->lbn = 0;
	root->height = 1;
	root->state = 0;
	root->left = NULL;
	root->right = NULL;
}

/**
 * Get the height of avl-tree
 @return height */
int
height(
    NVM_inode* N)
{
    if (N == NULL) {
        return 0;
    }
    return N->height;
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
    NVM_inode* N)
{
    if (N == NULL) {
        return 0;
    }

    return height(N->left) - height(N->right);
}

NVM_inode*
rightRotate(
    NVM_inode* y)
{
    NVM_inode* x = y->left;
    NVM_inode* T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = Max(height(y->left), height(y->right)) + 1;
    x->height = Max(height(x->left), height(x->right)) + 1;

    // Return new root
    return x;
}

NVM_inode*
leftRotate(
    NVM_inode* x)
{
    NVM_inode* y = x->right;
    NVM_inode* T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    x->height = Max(height(x->left), height(x->right)) + 1;
    y->height = Max(height(y->left), height(y->right)) + 1;

    // Return new root
    return y;
}
