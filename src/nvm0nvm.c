#include <stdio.h>
#include "nvm0common.h"

//NVM_metadata* NVM;

void
nvm_system_init()
{
    //TODO: get_nvm_address();

    //TODO:
    //if(setup required)
    //construct_nvm();

    //TODO:
    //else
    //start_ecovery();

    //TODO: start sync_thread
}

/**
 * Initialize and set up NVM */
void
init_nvm_address(
    void *start_addr)
{
    int i;
    
    // Initialize NVM address at the first time
    NVM = (NVM_metadata *)start_addr;
    NVM->VOL_TABLE_START = (VT_entry *)(NVM + 1);
    NVM->INODE_START     = (NVM_inode *)(NVM->VOL_TABLE_START + MAX_VT_ENTRY);
    NVM->DATA_START      = (char *)(NVM->INODE_START + MAX_NVM_INODE);
    
    // Initialize Free list for VT_entry
    VT_entry* vteptr = NVM->VOL_TABLE_START;
    NVM->FREE_VTE_LIST_HEAD = vteptr;
    for (i = 0; i < MAX_VT_ENTRY - 1; i++)
    {
        vteptr->vid = 0;
        vteptr->fd  = 0;
        vteptr->next = i + 1;
        vteptr++;
    }
    vteptr->vid = 0;
    vteptr->fd  = 0;
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

/**
 * Print NVM address information */
void
print_nvm_info()
{
    printf("\nHere is the NVM information \n");
    
    printf("\n[RESERVED AREA]\n");
    printf("- start  : %p\n", NVM);
    printf("- size   : %ld\n", sizeof(NVM_metadata));

    printf("\n[VOLUME TABLE]\n");
    printf("- start  : %p\n", NVM->VOL_TABLE_START);
    printf("- end    : %p\n", NVM->VOL_TABLE_START + MAX_VT_ENTRY);
    printf("- vte size  : %ld Bytes\n", sizeof(VT_entry));
    printf("- #vte      : %d\n", MAX_VT_ENTRY);
    printf("- free list : %p\n", NVM->FREE_VTE_LIST_HEAD);
    printf("- #free vte : %d\n", get_free_vte_num());
//  data_dump((unsigned char*)NVM->VOL_TABLE_START, 1024);

    printf("\n[INODE]\n");
    printf("- start : %p\n", NVM->INODE_START);
    printf("- end   : %p\n", NVM->INODE_START + MAX_NVM_INODE);
    printf("- inode size  : %ld Bytes\n", sizeof(NVM_inode));
    printf("- #inodes     : %d\n", MAX_NVM_INODE);
    printf("- free list   : %p\n", NVM->FREE_INODE_LIST_HEAD);
    printf("- #free inode : %d\n", get_free_inode_num());
    printf("- sync list   : %p\n", NVM->SYNC_INODE_LIST_HEAD);
    printf("- #sync inode : %d\n", get_sync_inode_num());
//  data_dump((unsigned char*)NVM->INODE_START, sizeof(NVM_inode)*20);
    
    printf("\n[DATA]\n");
    printf("- start : %p\n", NVM->DATA_START);
    printf("- end   : %p\n", NVM->DATA_START + MAX_NVM_INODE * BLOCK_SIZE);
    printf("- block size : %d Bytes\n", BLOCK_SIZE);
    printf("- #blk in nvm: %d\n", MAX_NVM_INODE - get_free_inode_num());
    printf("\n\n");
//  data_dump((unsigned char*)NVM->DATA_START, BLOCK_SIZE * 33);
}

/**
 * Get the number of not using vte from free-vte-list
 @return the number of free-vte */
int
get_free_vte_num()
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

/**
 * Get the number of not using inode from free-inode-list
 @return the number of free-inode */
int
get_free_inode_num()
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

/**
 * Get the number of inodes for syncing from sync-inode-list
 @return the number of inodes for syncing */
int
get_sync_inode_num()
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

/**
 * Get the index for given inode
 @return the index of that inode */
unsigned int 
get_nvm_inode_idx(
    NVM_inode* addr)
{
    return (unsigned int)(((char*)addr - (char*)NVM->INODE_START)/sizeof(NVM_inode));
}
