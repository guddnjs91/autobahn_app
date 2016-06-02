#ifndef _NVM_VOLUME_TABLE_H_
#define _NVM_VOLUME_TABLE_H_

#include "nvm0avltree.h"

/**
 * nvm0volumetable.h - header file for Volume Table */

/* Represent one volume table entry */
typedef struct _vt_entry {

    unsigned int        vid;    // volume id (implicit filename)
    int                 fd;     // file descriptor (useless when recovery)
    struct _tree_node*  root;  // inode AVL tree root

} VT_entry;

lfqueue<_vt_entry*>*   VTE_FREE_LFQUEUE;
lfqueue<_vt_entry*>*   VTE_INUSE_LFQUEUE;

#endif
