#include <stdio.h>
#include <stdlib.h>
#include "nvm0common.h"
#include "nvm0avltree.h"

/**
 * Search tree node object from avl tree root
 @return tree node with inode's lbn from avl-tree */
tree_node*
search_tree_node(
    tree_node* root,
    uint32_t lbn)
{
    tree_node* local_root = root;

    while(local_root != NULL) 
    {
        if(lbn == local_root->inode->lbn)
        {
            return local_root;
        }
        else if(lbn < local_root->inode->lbn) 
        {
            local_root = local_root->left;
        }
        else if(lbn > local_root->inode->lbn)
        {
            local_root = local_root->right;
        }
    }

    return nullptr;
}

/**
 * Insert tree node object to avl tree
 @return root node of avl-tree*/
tree_node*
insert_tree_node(
    tree_node* root,
    tree_node* node)
{
    if (root == nullptr)
    {
        root = node;
        return root;
    }

    if (node->inode->lbn < root->inode->lbn)
    {
        root->left = insert_tree_node(root->left, node);
    }
    else
    {
        root->right = insert_tree_node(root->right, node);
    }

    //update height
    root->height = max_height(get_height(root->left), get_height(root->right)) + 1;

    //rebalancing
    int balance = get_balance(root);
    
    // LL
    if (balance > 1 && node->inode->lbn < root->left->inode->lbn)
    {
        return right_rotate(root);
    }
    
    // LR
    if (balance > 1 && node->inode->lbn > root->left->inode->lbn)
    {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }   
    
    // RR
    if (balance < -1 && node->inode->lbn > root->right->inode->lbn)
    {
        return left_rotate(root);
    }
    
    // RL
    if (balance < -1 && node->inode->lbn < root->right->inode->lbn)
    {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}

/**
 * Delete tree node object from avl tree 
 * @return tree after deleting the node */
tree_node*
delete_tree_node(
    tree_node* root,  /* !<out: sub-tree root after deleting node */
    tree_node* node) /* !<in: node to be deleted */
{
    if (root == nullptr)
    {
        return root;
    }
    
    if (node->inode->lbn < root->inode->lbn)
    {
        root->left = delete_tree_node(root->left, node);
    }
    else if (node->inode->lbn > root->inode->lbn)
    {
        root->right = delete_tree_node(root->right, node);
    }
    else
    {
        // 1 child || no child
        if ((root->left == nullptr) || (root->right == nullptr))
        {
            tree_node* temp = root->left ? root->left : root->right;
            
            if (temp == nullptr)
            { // no child
                temp = root;
                root = nullptr;
            }
            else 
            { // 1 child, copy the contents of the non-empty child
                *root = *temp;
            }

            deallocate_tree_node(temp);
        } 
        else
        {
            // 2 children
            tree_node* temp =  min_value_node(root->right);
            
            // copy the inorder successor's data
            ///////////////////////////////////////////////////////////////////////////////////////
            root->inode->lbn = temp->inode->lbn;
            root->inode->state = temp->inode->state;
            root->inode->volume = temp->inode->volume;
            root->valid = temp->valid;
            ///////////////////////////////////////////////////////////////////////////////////////
            
            // Delete the inorder successor
            root->right = delete_tree_node(root->right, temp);
        }
    }
    
    if (root == nullptr)
    {
        return root;
    }
    
    //update height
    root->height = max_height(get_height(root->left),get_height(root->right)) + 1;
    
    //rebalancing
    int balance = get_balance(root);
    
    //LL
    if (balance > 1 && get_balance(root->left) >= 0)
    {
        return right_rotate(root);
    }
    
    //LR
    if (balance > 1 && get_balance(root->left) < 0)
    {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }
    
    //RR
    if (balance < -1 && get_balance(root->right) <= 0)
    {
        return left_rotate(root); 
    }
    
    //RL
    if (balance < -1 && get_balance(root->right) > 0)
    {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }
    
    return root;
}

/**
 * Find the minimum key from AVL tree.
 * @return tree node that has minimum key value in tree.*/
tree_node*
min_value_node(
    tree_node* node)
{
    tree_node* current = node;
    
    while (current->left != nullptr)
    {
        current = current->left;
    }
    
    return current;
}

tree_node*
allocate_tree_node(
    inode_entry* inode)
{
    tree_node* t = (tree_node*)malloc(sizeof(tree_node));
    t->inode = inode;
    t->valid = TREE_VALID;
    t->left = nullptr;
    t->right = nullptr;
    t->height = 1;
    
    return t;
}

/**
 * Deallocate tree's inode structure */
void
deallocate_tree_node(
    tree_node* node)
{
    // Re-initialization
    node->inode->lbn = 0;
    node->inode->state = INODE_STATE_FREE;
    node->inode->volume = nullptr;
    node->valid = TREE_INVALID;
    node->height = 1;
    node->left = nullptr;
    node->right = nullptr;
}

/**
 * Get the height of avl-tree
 @return height */
int
get_height(
    tree_node* node)
{
    if (node == NULL) 
    {
        return 0;
    }
    return node->height;
}

int
max_height(
    int a,
    int b)
{
    return a > b ? a : b;
}

int
get_balance(
    tree_node* node)
{
    if (node == NULL)
    {
        return 0;
    }

    return get_height(node->left) - get_height(node->right);
}

tree_node*
right_rotate(
    tree_node* y)
{
    tree_node* x = y->left;
    tree_node* T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = max_height(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_height(get_height(x->left), get_height(x->right)) + 1;

    // Return new root
    return x;
}

tree_node*
left_rotate(
    tree_node* x)
{
    tree_node* y = x->right;
    tree_node* T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    x->height = max_height(get_height(x->left), get_height(x->right)) + 1;
    y->height = max_height(get_height(y->left), get_height(y->right)) + 1;

    // Return new root
    return y;
}
