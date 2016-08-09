#include "nvm0common.h"

/**
 * Get inode_entry object for lbn.
 * @return inode */
inode_idx_t
get_inode_entry_idx(
    volume_entry* ve, /* !<in: volume that contains inodes we are looking for */
    uint32_t lbn)     /* !<in: find inode which has this lbn */
{
    inode_idx_t idx;
    hash_node* hash_node;
    inode_entry* inode;

    hash_node  = search_hash_node(ve->hash_table, lbn);

    if(hash_node != nullptr) {
        inode = hash_node->inode;
        idx = inode - nvm->inode_table;
        return idx;
    } else {
        idx = alloc_inode_entry_idx(lbn);
        inode = &nvm->inode_table[idx];
        inode->volume = ve;
        hash_node = new_hash_node(inode);
        insert_hash_node(ve->hash_table, hash_node);
    }

    return idx;
}

/**
 * Allocate new volume_entry from free-list of ve.
 * @return inode */
inode_idx_t
alloc_inode_entry_idx(
    uint32_t lbn) /* !<in: lbn to new allocating inode */
{
    /**
     * Get the free inode from free_inode_lfqueue.
     * Multi-writers can ask for each free-inode and
     * lf-queue deque(consume) free-inode to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
    inode_idx_t idx = inode_free_lfqueue->dequeue();

    /**
     * Setting up the acquired inode.
     * Give inode its lbn, and make its state as ALLOCATED */
    nvm->inode_table[idx].lbn = lbn;
    nvm->inode_table[idx].volume = nullptr;
    nvm->inode_table[idx].lock = PTHREAD_MUTEX_INITIALIZER;
    return idx;
}
