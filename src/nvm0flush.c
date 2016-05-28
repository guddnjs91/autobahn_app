#include <unistd.h>
#include <stdio.h>
#include "nvm0common.h"
#include "nvm0lfq.h"

extern NVM_metadata* NVM;

/**
 * Sync one data block from NVM to data */
void
nvm_sync()
{
    /**
     * Get the inode to be flushed soon.
     * This inode in sync_inode_lfqueue is dirty so
     * flushing and re-cycling it should be handled pro-actively */
    NVM_inode* inode = sync_inode_lfqueue.dequeue();

    /**
     * Get the index of inode which is also the index of data block,
     * write and flush that data block from NVM to storage. */
    unsigned int idx = get_nvm_inode_idx(inode);
    write(inode->vte->fd, NVM->DATA_START + BLOCK_SIZE * idx, BLOCK_SIZE);
    fsync(inode->vte->fd);
    inode->state = INODE_STATE_SYNCED;
    printf("sync %d Bytes\n", BLOCK_SIZE);

    /**
     * Once flushed inode should be reclaimed for re-use.
     * This inode temporarily enqueued to flush_inode_lfqueue. */
    inode->state = INODE_STATE_FREE;
    flush_inode_lfqueue.enqueue(*inode);
}

/**
 * Deallocate flushed inode : delete inode from vte's tree and
 * return it to the free_inode_lfqueue for next use. */
void
dealloc_nvm_inode(NVM_inode* inode) /* !<out: inode to be deallocated */
{
    // delete inode from vte->iroot first.
    delete_nvm_inode(inode); // implemented with nvm0avltree.c (later)

    // re-initialize inode.
    inode->left = NULL;
    inode->right = NULL;
    inode->height = 0;
    inode->state = INODE_STATE_FREE;

    // enqueue inode to free_inode_lfqueue.
    free_inode_lfqueue.enqueue(*inode);

}
