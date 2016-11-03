#include <fcntl.h>
#include <string>
#include "nvm0nvm.h"
#include "nvm0hash.h"
#include "nvm0volume.h"
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>

volume_entry *get_volume_entry(volume_idx_t v_idx)
{
    return &nvm->volume_table[v_idx];
}

/**
 * Finds/creates a volume entry with vid and gets an index of the volume entry.
 *
 * Searches through a volume entry with the vid.
 * If found, return the index of the volume entry.
 * If failed, allocate a new volume entry with vid and returns the index.
 *
 * @return an index of a volume entry with vid.
 */
volume_idx_t get_volume_entry_idx(uint32_t vid)
{
    for (volume_idx_t v_idx = 0; v_idx < nvm->max_volume_entry; v_idx++) {
        if (nvm->volume_table[v_idx].vid == vid) {
            return v_idx;
        }
    }
    
    /* runs below if search above failed */

    return alloc_volume_entry_idx(vid);
}

/**
 * Allocates a new volume_entry from the free volume list.
 *
 * @return index of the allocated volume entry.
 */
volume_idx_t alloc_volume_entry_idx(uint32_t vid)
{
    volume_idx_t free_v_idx;
    if (volume_free_lfqueue->is_empty()){
        // TODO: if free volume entry is not available, 
        // need to flush out some volume entries in use.
        fprintf(stderr, "volume free queue need to be flushed\n");
        assert(0);
        // temporary protection
    } else {
        free_v_idx = volume_free_lfqueue->dequeue();
    }

    volume_entry *ve = get_volume_entry(free_v_idx);
    ve->vid = vid;
    ve->fd  = open(get_filename(vid), O_DIRECT | O_RDWR | O_CREAT, 0644);
    if (ve->fd < 0) {
        printf("Opening file: failed\n");
        printf("Error no is : %d\n", errno);
        printf("Error description : %s\n", strerror(errno));
        assert(0);
    }
    ve->hash_table  = new_hash_table();

    volume_inuse_lfqueue->enqueue(free_v_idx);

    return free_v_idx;
}

/**
 * Get filename with argument vid (vid maps filename 1 to 1)
 * 
 * @return the file name for vid
 */
const char *get_filename(uint32_t vid)
{
    std::string filename;

#ifdef MULTIPLE_STORAGE_ON
    if (vid % 2 == 1) {
        filename = "/opt/nvm1/NVM/VOL_";
        filename += std::to_string(vid);
        filename += ".txt";
    } else {
        filename = "/opt/nvm2/NVM/VOL_";
        filename += std::to_string(vid);
        filename += ".txt";
    }
#else
        filename = "./VOL_";
        filename += std::to_string(vid);
        filename += ".txt";
#endif

    return filename.c_str();
}

off_t get_filesize(uint32_t vid)
{
    struct stat st;
    if (stat(get_filename(vid), &st) == 0) {
        return st.st_size;
    } else {
        fprintf(stderr, "Cannot determine size of %s: %s\n", get_filename(vid), strerror(errno));
        assert(0);
    }
}
