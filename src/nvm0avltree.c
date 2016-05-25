#include <stdio.h>
#include "nvm0common.h"

extern NVM_metadata* NVM;

/* Search NVM_inode object from avl tree root */
NVM_inode* search_nvm_inode(NVM_inode* root, unsigned int lbn)
{
	NVM_inode* localRoot = root;

	while (localRoot != NULL)
	{
		if (lbn == localRoot->lbn)
			return localRoot;

		else if (lbn < localRoot->lbn)
			localRoot = localRoot->left;

		else if (lbn > localRoot->lbn)
			localRoot = localRoot->right;
	}

	return NULL;
}

/* Insert NVM_inode object to avl tree */
NVM_inode* insert_nvm_inode(NVM_inode* root, NVM_inode* inode)
{
	if (root == NULL)
	{
		root = inode;
		return root;
	}

	if (inode->lbn < root->lbn)
		root->left = insert_nvm_inode(root->left, inode);
	else
		root->right = insert_nvm_inode(root->right, inode);

	//update height
	root->height = Max(height(root->left), height(root->right)) + 1;

	//rebalancing
	int balance = getBalance(root);
	
	// LL
	if (balance > 1 && inode->lbn < root->left->lbn)
		return rightRotate(root);
	
	// LR
	if (balance > 1 && inode->lbn > root->left->lbn)
	{
		root->left = leftRotate(root->left);
		return rightRotate(root);
	}	
	
	// RR
	if (balance < -1 && inode->lbn > root->right->lbn)
		return leftRotate(root);
	
	// RL
	if (balance < -1 && inode->lbn < root->right->lbn)
	{
		root->right = rightRotate(root->right);
		return leftRotate(root);
	}

	return root;
}

int height(NVM_inode* N)
{
	if (N == NULL)
		return 0;
	return N->height;
}

int Max(int a, int b) { return a > b ? a : b; }

int getBalance(NVM_inode* N)
{
	if (N == NULL)
		return 0;

	return height(N->left) - height(N->right);
}

NVM_inode* rightRotate(NVM_inode* y)
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

NVM_inode* leftRotate(NVM_inode* x)
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
