#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stack>
#include "nvm0common.h"

/**
Balloon thread function wakes up when free inode shortage.*/
void*
balloon_thread_func(
    void* data)
{
    while(sys_terminate == 0)
    {
        nvm_balloon();
    }

    return NULL;
}

/**
Balloon function traverses tree and reclaims free inodes
in invalid tree nodes. */
void
nvm_balloon(
    void)
{
    /* Wait signal from nvm_write() function. */
    pthread_mutex_lock(&g_balloon_mutex);
    pthread_cond_wait(&g_balloon_cond, &g_balloon_mutex);
    pthread_mutex_unlock(&g_balloon_mutex);

    /* Lock write-lock to be mutually exclusive to write threads. */
    pthread_rwlock_wrlock(&g_balloon_rwlock);

    /* Traverse volume table that each entry has one tree structure. */
    for(volume_idx_t v = 0;
        nvm->volume_table[v].vid != 0 &&
        v < nvm->max_volume_entry; v++)
    {
        tree_root* tree = nvm->volume_table[v].tree;
        std::stack<tree_node*> tnode_stack;
        tnode_stack.push(tree->root);

        /* Preorder traverse tree. */
        while(!tnode_stack.empty())
        {
            tree_node* tnode = tnode_stack.top();
            tnode_stack.pop();

            if(tnode->inode->state == INODE_STATE_CLEAN)
            {
                /* Logical delete tree node. */
                tnode->valid = TREE_INVALID;
                tree->count_invalid++;

                /* Reclaim to free inode LFQ. */
                inode_idx_t idx = (inode_idx_t)(tnode->inode - nvm->inode_table);
                inode_free_lfqueue->enqueue(idx);
            }

            if(tnode->right != nullptr)
            {
                tnode_stack.push(tnode->right);
            }
            if(tnode->left != nullptr)
            {
                tnode_stack.push(tnode->left);
            }
        }
    }


    /* Unlock write-lock. */
    pthread_rwlock_unlock(&g_balloon_rwlock);
}
