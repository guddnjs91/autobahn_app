/**
 * nvm0volume.h - header file for Volume Table */

#ifndef nvm0volume_h
#define nvm0volume_h

/* Represent one volume table entry */
struct volume_entry {
    uint32_t            vid;    // volume id (implicit filename)
    int                 fd;     // file descriptor (useless when recovery)
    struct tree_node*   root;   // inode AVL tree root
};

lfqueue<volume_entry*>* volume_free_queue;
lfqueue<volume_entry*>* volume_inuse_queue;

#endif
