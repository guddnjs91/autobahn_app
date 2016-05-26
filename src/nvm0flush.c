#include <unistd.h>
#include <stdio.h>
#include "nvm0common.h"

extern NVM_metadata* NVM;

/* Sync one data block from NVM to data */
void
nvm_sync()
{
    // get the first inode in sync-list
    NVM_inode* inode = __sync_lock_test_and_set(
                &NVM->SYNC_INODE_LIST_HEAD, NVM->SYNC_INODE_LIST_HEAD->s_next);

    // get the index and write that data block
    unsigned int idx = get_nvm_inode_idx(inode);
    write(inode->vte->fd, NVM->DATA_START + BLOCK_SIZE * idx, BLOCK_SIZE);
    inode->state = INODE_STATE_SYNCED;
    printf("sync %d Bytes\n", BLOCK_SIZE);
}
