/**
 * nvm0volumetable.h - header file for Volume Table */

/* Represent one volume table entry */
typedef struct _vt_entry {

    unsigned int        vid;    // volume id (implicit filename)
    int                 fd;     // file descriptor (useless when recovery)
    struct _nvm_inode*  iroot;  // inode root
    
    // managed by vte's free-list
    unsigned int next;

} VT_entry;

lfqueue<_vt_entry*>*   VTE_FREE_LFQUEUE;
lfqueue<_vt_entry*>*   VTE_INUSE_LFQUEUE;

