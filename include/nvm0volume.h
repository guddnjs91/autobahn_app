/**
 * nvm0volume.h - header file for Volume Table */

#ifndef nvm0volume_h
#define nvm0volume_h

typedef uint32_t volume_idx_t;

/* Represent one volume table entry */
struct volume_entry {
    uint32_t            vid;    // volume id (implicit filename)
    int                 fd;     // file descriptor (useless when recovery)
    struct tree_root*   tree;   // inode AVL tree root
};

extern lfqueue<volume_idx_t>* volume_free_lfqueue;
extern lfqueue<volume_idx_t>* volume_inuse_lfqueue;

#endif
