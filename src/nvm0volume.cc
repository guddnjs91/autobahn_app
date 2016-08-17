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
 * Search for a volume_entry object from Volume Table.
 * @return found entry */
volume_idx_t
search_volume_entry_idx(
    uint32_t vid)     /* !<in: searching tree with vid */
{
    volume_idx_t idx;

    for(idx = 0; idx < nvm->max_volume_entry; idx++) {
        if(nvm->volume_table[idx].vid == vid) {
            break;
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
    volume_idx_t idx = volume_free_lfqueue->dequeue();

    nvm->volume_table[idx].vid = vid;
    nvm->volume_table[idx].fd = open(get_filename(vid), O_RDWR| O_CREAT, 0644);
    nvm->volume_table[idx].hash_table = new_hash_table();

    return idx;
}

/**
 * Get filename with argument vid (vid maps filename 1 to 1)
 @return const char* containing filename */
const char*
get_filename(
    uint32_t vid) /* !<in: vid representing its own filename */
{
    std::string filename;

    if(vid % 2 == 1)
    {
        filename = "/opt/nvm1/NVM/VOL_";
        filename += std::to_string(vid);
        filename += ".txt";
    }

    else
    {
        filename = "/opt/nvm2/NVM/VOL_";
        filename += std::to_string(vid);
        filename += ".txt";
    }

    return filename.c_str();
}
