#include <unistd.h>
#include <stdio.h>
#include "nvm0common.h"
#include "nvm0lfq.h"

extern NVM_metadata* NVM;

/**
 * Sync one data block from NVM to data */
void
nvm_flush()
{
    /**
     * Get the inode to be flushed soon.
     * This inode in sync_inode_lfqueue is dirty so
     * flushing and re-cycling it should be handled pro-actively */
    NVM_inode* inode = sync_inode_lfqueue.dequeue();

    // get the index and write that data block
    unsigned int idx = get_nvm_inode_idx(inode);
    write(inode->vte->fd, NVM->DATA_START + BLOCK_SIZE * idx, BLOCK_SIZE);
    fsync(inode->vte->fd);
    inode->state = INODE_STATE_SYNCED;
    printf("sync %d Bytes\n", BLOCK_SIZE);

    /**
     * Once flushed inode should be reclaimed for re-use.
     * This inode returns to the free_inode_lfqueue. */
    inode->state = INODE_STATE_FREE;
    free_inode_lfqueue.enqueue(*inode);
}
