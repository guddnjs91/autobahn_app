#include <fcntl.h>
#include "nvm0common.h"

//private function declarations
const char* get_filename(uint32_t vid);

/**
 * Get volume_entry object which has vid.
 * @return a free volume entry */
volume_idx_t
get_volume_entry_idx(
    uint32_t vid) /* !<in: key to find volume_entry */
{
    volume_idx_t idx = search_volume_entry_idx(vid);
    
    if(idx == nvm->max_volume_entry) {
        idx = alloc_volume_entry_idx(vid);
    }
    
    return idx;
}

/**
 * Search volume_entry object from Volume Table.
 * @return found entry */
volume_idx_t
search_volume_entry_idx(
    uint32_t vid)     /* !<in: searching tree with vid */
{
    volume_idx_t idx;

    for(idx = 0; idx < nvm->max_volume_entry; idx++) 
    {
        if(nvm->volume_table[idx].vid == vid)
        {
            return idx;
        }
    }

    return idx;
}

/**
 * Allocate new volume_entry from volume_free_lfqueue
 * @return allocated volume entry */
volume_idx_t
alloc_volume_entry_idx(
    uint32_t vid) /* !<in: given vid to new allocated volume_entry */
{
    /**
     * Get the free ve from volume_free_lfqueue.
     * Multi-writers can ask for each free volume entry and
     * lfqueue dequeue(consume) free volume entry to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
     volume_idx_t idx = volume_free_lfqueue->dequeue();

    /**
     * Setting up acquired free volume entry for use now.
     * Give ve its vid, fd from opening the file.
     * Initialize hash table 
     * the logical blocks of inodes for its volume(file) */
    nvm->volume_table[idx].fd = open(get_filename(vid), O_RDWR| O_CREAT, 0644);
    nvm->volume_table[idx].hash_table = new_hash_table();
    nvm->volume_table[idx].vid = vid;

    return idx;
}

/**
 * Get filename with argument vid (vid maps filename 1 to 1)
 @return const char* containing filename */
const char*
get_filename(
    uint32_t vid) /* !<in: vid representing its own filename */
{
    std::string filename = "VOL_";
    filename += std::to_string(vid);
    filename += ".txt";

    return filename.c_str();
}
