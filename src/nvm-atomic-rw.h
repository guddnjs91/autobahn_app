#ifndef __NVM_ATOMIC_RW_H_
#define __NVM_ATOMIC_RW_H_

#include <pthread.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include "dumpcode.h"

#define MAX_VT_ENTRY        1024
#define MAX_NVM_INODE       2000000
#define BLOCK_SIZE          512

#define	INODE_STATE_FREE        0
#define	INODE_STATE_ALLOCATED   1
#define	INODE_STATE_WRITTEN     2
#define INODE_STATE_SYNCED      3

/* Represent one inode object */
typedef struct _nvm_inode {
    unsigned int lbn;           // logical block number
    int state;                  // state of block
    struct _vt_entry* vte;      // volume id (implicit filename)

    unsigned int f_prev;        // for free list
    unsigned int f_next;        // for free list
    
    struct _nvm_inode* s_prev;  // for sync list => Is there any way to represent as index offset??
    struct _nvm_inode* s_next;  // for sync list
    
    struct _nvm_inode* left;    // for AVL tree
    struct _nvm_inode* right;   // for AVL tree
    int height;                 // for AVL tree
    
} NVM_inode;

/* Represent one volume table entry */
typedef struct _vt_entry {
    unsigned int vid;           // volume id (implicit filename)
    int fd;                     // file descriptor (useless when recovery)
    struct _nvm_inode* iroot;   // inode root
    
    unsigned int next;          // managed by free list
} VT_entry;

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
    struct _nvm_inode*	FREE_INODE_LIST_HEAD;
    struct _nvm_inode*	FREE_INODE_LIST_TAIL;

    // Sync list for NVM_inode
    struct _nvm_inode*	SYNC_INODE_LIST_HEAD;
    struct _nvm_inode*	SYNC_INODE_LIST_TAIL;
} NVM_metadata;

NVM_metadata* NVM;

/* Initialize NVM */
void init_nvm_address(void *start_addr)
{
    int i;
    
    // Initialize NVM address at the first time
    NVM = (NVM_metadata *)start_addr;
    NVM->VOL_TABLE_START        = (VT_entry *)(NVM + 1);
    NVM->INODE_START            = (NVM_inode *)(NVM->VOL_TABLE_START + MAX_VT_ENTRY);
    NVM->DATA_START             = (char *)(NVM->INODE_START + MAX_NVM_INODE);
    
    // Initialize Free list for VT_entry
    VT_entry* vteptr = NVM->VOL_TABLE_START;
    NVM->FREE_VTE_LIST_HEAD = vteptr;
    for (i = 0; i < MAX_VT_ENTRY - 1; i++)
    {
        vteptr->vid = 0;
        vteptr->next = i + 1;
        vteptr++;
    }
    vteptr->vid = 0;
    vteptr->next = MAX_VT_ENTRY;
    NVM->FREE_VTE_LIST_TAIL = vteptr;
    
    // Initialize Free list for NVM_inode
    NVM_inode* iptr = NVM->INODE_START;
    NVM->FREE_INODE_LIST_HEAD = iptr;
    for (i = 0; i < MAX_NVM_INODE - 1; i++) 
    {
        iptr->lbn = 0;
        iptr->f_next = i + 1;
        iptr->state = INODE_STATE_FREE;
        iptr++;
    }
    iptr->lbn = 0;
    iptr->f_next = MAX_NVM_INODE;
    iptr->state = INODE_STATE_FREE;
    NVM->FREE_INODE_LIST_TAIL = iptr;

    // Initialize Sync List for NVM_inode
    NVM->SYNC_INODE_LIST_HEAD = NULL;
    NVM->SYNC_INODE_LIST_TAIL = NULL;
}

int get_free_vte_num()
{
    int cnt = 0;
    VT_entry* vte = NVM->FREE_VTE_LIST_HEAD;
    while(vte != NVM->FREE_VTE_LIST_TAIL)
    {
        cnt++;
        vte = NVM->VOL_TABLE_START + vte->next;
    }
    
    cnt++;
    
    return cnt;
}

int get_free_inode_num()
{
	int cnt = 0;
	NVM_inode* inode = NVM->FREE_INODE_LIST_HEAD;
	while(inode != NVM->FREE_INODE_LIST_TAIL)
	{
		cnt++;
		inode = NVM->INODE_START + inode->f_next;
	}

	cnt++;

	return cnt;
}

int get_sync_inode_num()
{
	int cnt = 0;
	NVM_inode* inode = NVM->SYNC_INODE_LIST_HEAD;
	
	if(inode == NULL)
		return cnt;

	while(inode != NVM->SYNC_INODE_LIST_TAIL)
	{
		cnt++;
		inode = inode->s_next;
	}

	cnt++;

	return cnt;
}
// Print NVM address information
void print_nvm_info()
{
	printf("\nHere is the NVM information \n");
	
	printf("\n[RESERVED AREA]\n");
	printf("- start  : %p\n", NVM);
	printf("- size   : %ld\n", sizeof(NVM_metadata));

	printf("\n[VOLUME TABLE]\n");
	printf("- start	 : %p\n", NVM->VOL_TABLE_START);
	printf("- end    : %p\n", NVM->VOL_TABLE_START + MAX_VT_ENTRY);
	printf("- vte size  : %ld Bytes\n", sizeof(VT_entry));
	printf("- #vte      : %d\n", MAX_VT_ENTRY);
	printf("- free list : %p\n", NVM->FREE_VTE_LIST_HEAD);
	printf("- #free vte : %d\n", get_free_vte_num());
//	data_dump((unsigned char*)NVM->VOL_TABLE_START, 1024);

	printf("\n[INODE]\n");
	printf("- start : %p\n", NVM->INODE_START);
	printf("- end   : %p\n", NVM->INODE_START + MAX_NVM_INODE);
	printf("- inode size  : %ld Bytes\n", sizeof(NVM_inode));
	printf("- #inodes     : %d\n", MAX_NVM_INODE);
	printf("- free list   : %p\n", NVM->FREE_INODE_LIST_HEAD);
	printf("- #free inode : %d\n", get_free_inode_num());
	printf("- sync list   : %p\n", NVM->SYNC_INODE_LIST_HEAD);
	printf("- #sync inode : %d\n", get_sync_inode_num());
//	data_dump((unsigned char*)NVM->INODE_START, sizeof(NVM_inode)*20);
	
	printf("\n[DATA]\n");
	printf("- start : %p\n", NVM->DATA_START);
	printf("- end   : %p\n", NVM->DATA_START + MAX_NVM_INODE * BLOCK_SIZE);
	printf("- block size : %d Bytes\n", BLOCK_SIZE);
	printf("- #blk in nvm: %d\n", MAX_NVM_INODE - get_free_inode_num());
	printf("\n\n");
//	data_dump((unsigned char*)NVM->DATA_START, BLOCK_SIZE * 33);
}

const char* get_filename(unsigned int vid)
{
    std::string filename = "VOL";
    std::stringstream ss;
    ss << vid;
    filename += ss.str();
    filename += ".txt";

    return filename.c_str();
}

/* Allocate new VT_entry from free-list of vte */
VT_entry* alloc_vt_entry(unsigned int vid)
{
	VT_entry* vte;

	// Check if free-list is empty
	if(NVM->FREE_VTE_LIST_HEAD != NVM->FREE_VTE_LIST_TAIL)
	{
		vte = __sync_lock_test_and_set(
				&NVM->FREE_VTE_LIST_HEAD, NVM->VOL_TABLE_START + NVM->FREE_VTE_LIST_HEAD->next);
	}
	else
	{
		// wait until vt_entry freed (later)
	}

	vte->vid = vid;
        vte->fd = open(get_filename(vid), O_WRONLY| O_CREAT, 0644);
	vte->next = MAX_VT_ENTRY;	// meaning next null
	vte->iroot = NULL;

	return vte;
}

VT_entry* search_vt_entry(VT_entry* vt_root, unsigned int vid)
{
	// Just linear search in this code
	// need to search from tree structure later
	int i;
	VT_entry* vte = NVM->VOL_TABLE_START;

	for(i = 0; i < MAX_VT_ENTRY; i++) {
		if(vte->vid == vid)
			return vte;
		vte++;
	}

	
	return NULL;
}

/* Get VT_entry object for vid */
VT_entry* get_vt_entry(unsigned int vid)
{
	VT_entry* vte;

	// 1. search from vt_tree
	vte = search_vt_entry(NVM->VOL_TABLE_START, vid);
	
	// 2. If search found vte, return it
	if(vte != NULL)
	{
		return vte;
	}
	else
	{
		// 3. If not found, allocate new vte
		vte = alloc_vt_entry(vid);
		// insert this vte to vte_tree structure (later)
	}

	return vte;
}

int height(NVM_inode* N)
{
	if (N == NULL)
		return 0;
	return N->height;
}

int Max(int a, int b) { return a > b ? a : b; }

int getBalance(NVM_inode* N)
{
	if (N == NULL)
		return 0;

	return height(N->left) - height(N->right);
}

NVM_inode* rightRotate(NVM_inode* y)
{
	NVM_inode* x = y->left;
	NVM_inode* T2 = x->right;

	// Perform rotation
	x->right = y;
	y->left = T2;

	// Update heights
	y->height = Max(height(y->left), height(y->right)) + 1;
	x->height = Max(height(x->left), height(x->right)) + 1;

	// Return new root
	return x;
}

NVM_inode* leftRotate(NVM_inode* x)
{
	NVM_inode* y = x->right;
	NVM_inode* T2 = y->left;

	// Perform rotation
	y->left = x;
	x->right = T2;

	// Update heights
	x->height = Max(height(x->left), height(x->right)) + 1;
	y->height = Max(height(y->left), height(y->right)) + 1;

	// Return new root
	return y;
}

/* Allocate new VT_entry from free-list of vte */
NVM_inode* alloc_nvm_inode(unsigned int lbn)
{
	NVM_inode* inode;

	// Check if free-list is empty
	if(NVM->FREE_INODE_LIST_HEAD != NVM->FREE_INODE_LIST_TAIL)
	{
		inode = __sync_lock_test_and_set(
				&NVM->FREE_INODE_LIST_HEAD, NVM->INODE_START + NVM->FREE_INODE_LIST_HEAD->f_next);
	}
	else
	{
		// wait until vt_entry freed (later)
	}

	inode->lbn = lbn;
	inode->f_next = MAX_NVM_INODE;	// meaning NULL
	inode->s_prev = NULL;			// meaning NULL
	inode->s_next = NULL;			// meaning NULL
	inode->state = INODE_STATE_ALLOCATED;

	return inode;
}

NVM_inode* search_nvm_inode(NVM_inode* root, unsigned int lbn)
{
	NVM_inode* localRoot = root;

	while (localRoot != NULL)
	{
		if (lbn == localRoot->lbn)
			return localRoot;

		else if (lbn < localRoot->lbn)
			localRoot = localRoot->left;

		else if (lbn > localRoot->lbn)
			localRoot = localRoot->right;
	}

	return NULL;
}

NVM_inode* insert_nvm_inode(NVM_inode* root, NVM_inode* inode)
{
	if (root == NULL)
	{
		root = inode;
		return root;
	}

	if (inode->lbn < root->lbn)
		root->left = insert_nvm_inode(root->left, inode);
	else
		root->right = insert_nvm_inode(root->right, inode);

	//update height
	root->height = Max(height(root->left), height(root->right)) + 1;

	//rebalancing
	int balance = getBalance(root);
	
	// LL
	if (balance > 1 && inode->lbn < root->left->lbn)
		return rightRotate(root);
	
	// LR
	if (balance > 1 && inode->lbn > root->left->lbn)
	{
		root->left = leftRotate(root->left);
		return rightRotate(root);
	}	
	
	// RR
	if (balance < -1 && inode->lbn > root->right->lbn)
		return leftRotate(root);
	
	// RL
	if (balance < -1 && inode->lbn < root->right->lbn)
	{
		root->right = rightRotate(root->right);
		return leftRotate(root);
	}

	return root;
}

/* Get NVM_inode object for lbn*/
NVM_inode* get_nvm_inode(VT_entry* vte, unsigned int lbn)
{
	NVM_inode* inode;

	// 1. Search from inode tree
	inode = search_nvm_inode(vte->iroot, lbn);

	// 2. If search found inode, return it
	if(inode != NULL)
	{
		return inode;
	}
	else
	{
		// 3. If not found, allocate new inode
		inode = alloc_nvm_inode(lbn);
                inode->vte = vte;
		vte->iroot = insert_nvm_inode(vte->iroot, inode);
	}

	return inode;
}

unsigned int get_nvm_inode_idx(NVM_inode* addr)
{
	return (unsigned int)(((char*)addr - (char*)NVM->INODE_START)/sizeof(NVM_inode));
}

void insert_sync_inode_list(NVM_inode* inode)
{
	if(NVM->SYNC_INODE_LIST_HEAD == NULL)
	{
		NVM->SYNC_INODE_LIST_HEAD = inode;
		NVM->SYNC_INODE_LIST_TAIL = inode;
	}
	else
	{
		 inode->s_prev = __sync_lock_test_and_set(
				 &NVM->SYNC_INODE_LIST_TAIL, inode);
		 inode->s_prev->s_next = inode;
	}


}


void nvm_sync()
{
    NVM_inode* inode = __sync_lock_test_and_set(
                &NVM->SYNC_INODE_LIST_HEAD, NVM->SYNC_INODE_LIST_HEAD->s_next);

    unsigned int idx = get_nvm_inode_idx(inode);
    write(inode->vte->fd, NVM->DATA_START + BLOCK_SIZE * idx, BLOCK_SIZE);
    inode->state = INODE_STATE_SYNCED;
    // printf("sync %d Bytes\n", BLOCK_SIZE);
}

void nvm_atomic_write(unsigned int vid, unsigned int ofs, void* ptr, unsigned int len)
{
//	printf("NVM write %u Bytes to VOL%u.txt\n", len, vid);

	// Search VT_entry of vid
	VT_entry* vte = get_vt_entry(vid);
	
        // Calculate total write size by offset and length
	unsigned int lbn_start	= ofs / BLOCK_SIZE;
	unsigned int lbn_end	= (ofs + len) / BLOCK_SIZE;
	NVM_inode* inode;

	unsigned int write_bytes = 0;
	unsigned int offset = ofs % BLOCK_SIZE;
	for(unsigned int i = 0; i < lbn_end - lbn_start + 1; i++)
	{
		// Get NVM_inode with its lbn
		inode = get_nvm_inode(vte, lbn_start + i);
		
		// need lock?

		// Get how many bytes to write
		write_bytes = (len > BLOCK_SIZE - offset) ? (BLOCK_SIZE - offset) : len;
		
		// Write data to the NVRAM data block space
		unsigned int idx = get_nvm_inode_idx(inode);
		char* data_dst = NVM->DATA_START + BLOCK_SIZE * idx + offset; 
		memcpy(data_dst, ptr, write_bytes);
		inode->state = INODE_STATE_WRITTEN;

//		printf("Data Written %d Bytes to %p\n", write_bytes, data_dst);

		// Insert written inode to sync list
		insert_sync_inode_list(inode);

		// Re-inintialize offset and len
		offset = 0;
		len -= write_bytes;
	}
}

#endif
