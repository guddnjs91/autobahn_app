#ifndef __NVM_COMMON_H_
#define __NVM_COMMON_H_

#define MAX_VT_ENTRY        1024
#define MAX_NVM_INODE       2000000
#define BLOCK_SIZE          512

/* State of inode */
#define INODE_STATE_FREE        0
#define INODE_STATE_ALLOCATED   1
#define INODE_STATE_WRITTEN     2
#define INODE_STATE_SYNCED      3

/* Represent one volume table entry */
typedef struct _vt_entry {

    unsigned int        vid;    // volume id (implicit filename)
    int                 fd;     // file descriptor (useless when recovery)
    struct _nvm_inode*  iroot;  // inode root
    
    // managed by vte's free-list
    unsigned int next;

} VT_entry;

/* Represent one inode object */
typedef struct _nvm_inode {
    
    unsigned int        lbn;      // logical block number
    int                 state;    // state of block
    struct _vt_entry*   vte;      // volume id (implicit filename)

    // managed by inode's free-list
    unsigned int        f_prev;
    unsigned int        f_next;
    
    // managed by inode's sync-list
    // => Is there any way to represent as index offset??
    struct _nvm_inode*  s_prev;
    struct _nvm_inode*  s_next;
    
    // managed by vte's AVL tree
    struct _nvm_inode*  left;
    struct _nvm_inode*  right;
    int                 height;
    
} NVM_inode;

/* Represent the metadata of NVM */
typedef struct _nvm_metadata {

    // Base address for calculating offset
    struct _vt_entry*   VOL_TABLE_START;
    struct _nvm_inode*  INODE_START;
    char*               DATA_START;
    
    // Free list for VT_entry
    struct _vt_entry*   FREE_VTE_LIST_HEAD;
    struct _vt_entry*   FREE_VTE_LIST_TAIL;
    
    // Free list for NVM_inode
    struct _nvm_inode*  FREE_INODE_LIST_HEAD;
    struct _nvm_inode*  FREE_INODE_LIST_TAIL;

    // Sync list for NVM_inode
    struct _nvm_inode*  SYNC_INODE_LIST_HEAD;
    struct _nvm_inode*  SYNC_INODE_LIST_TAIL;

} NVM_metadata;

/* Global variable that can access every NVM area */
//NVM_metadata* NVM;

/* Common functions declaration */

void init_nvm_address(void *start_addr);
void print_nvm_info(void);
int get_free_vte_num(void);
int get_free_inode_num(void);
int get_sync_inode_num(void);
unsigned int get_nvm_inode_idx(NVM_inode* addr);

void nvm_atomic_write(unsigned int vid, unsigned int ofs, void* ptr, unsigned int len);
VT_entry* get_vt_entry(unsigned int vid);
VT_entry* search_vt_entry(VT_entry* vt_root, unsigned int vid);
VT_entry* alloc_vt_entry(unsigned int vid);
NVM_inode* get_nvm_inode(VT_entry* vte, unsigned int lbn);
NVM_inode* alloc_nvm_inode(unsigned int lbn);
void insert_sync_inode_list(NVM_inode* inode);
const char* get_filename(unsigned int vid);

NVM_inode* search_nvm_inode(NVM_inode* root, unsigned int lbn);
NVM_inode* insert_nvm_inode(NVM_inode* root, NVM_inode* inode);
int Max(int a, int b);
int height(NVM_inode* N);
int getBalance(NVM_inode* N);
NVM_inode* rightRotate(NVM_inode* y);
NVM_inode* leftRotate(NVM_inode* y);

void nvm_sync(void);

#endif
