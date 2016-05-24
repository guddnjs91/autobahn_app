#ifndef __NVM_ATOMIC_RW_H_
#define __NVM_ATOMIC_RW_H_

#include <pthread.h>

#define MAX_VT_ENTRY		1024
#define MAX_NVM_METADATA	2000000
#define BLOCK_SIZE 			512

#define	FLAG_FREE			0
#define	FLAG_ALLOCATED		1
#define	FLAG_WRITTEN		2

typedef struct _vt_entry {
	unsigned int vid;   	// volume id (file name)
	unsigned int vhead; 	// first mdata of volume
} VT_entry;

typedef struct _nvm_metadata {
	unsigned int lbn; 	// logical block number
	unsigned int next;	// next block of volume
	char flag;			// state of block

} NVM_metadata;


VT_entry *			NVM_VOL_TABLE_START;
NVM_metadata * 		NVM_METADATA_START;
char *				NVM_DATA_START;

unsigned int		VT_ENTRY_COUNTER;
NVM_metadata*		NVM_FREE_LIST_HEAD;

pthread_mutex_t		VT_LOCK;

/* Initialize NVM */
void init_nvm_address(void *start_addr)
{
	int i;

	// Initialize NVM address at the first time
	NVM_VOL_TABLE_START = (VT_entry *)start_addr;
	NVM_METADATA_START = (NVM_metadata *)(NVM_VOL_TABLE + MAX_VT_ENTRY);
	NVM_DATA_START = (char *)(NVM_METADATA_START + MAX_NVM_METADATA);
	
	// Initialize Volume Table Entry 
	VT_entry* veptr = NVM_VOL_TABLE_START;
	for(i = 0; i < MAX_VT_ENTRY; i++)
	{
		veptr->vid = 0;
		veptr++;
	}

//	pthread_mutex_init(&VT_LOCK, NULL);

	// Initialize free-list of metadata
	NVM_metadata* mdptr = NVM_METADATA_START;
	for(i = 0; i < MAX_NVM_METADATA; i++) 
	{
		mdptr->flag = FLAG_FREE;
		mdptr->next = i + 1;
		mdptr++;
	}

	NVM_FREE_LIST_HEAD = NVM_METADATA_START;
}

// Print NVM address information
void print_nvm_info()
{
	printf("---------------[NVM INFO]---------------------\n");
	printf("VOLUME TABLE :\n");
	printf("- start	 : %p\n", NVM_VOL_TABLE_START);
	printf("- end    : %p\n", NVM_VOL_TABLE_START + MAX_VT_ENTRY);
	printf("- #entry : %d\n", MAX_VT_ENTRY);
	printf("inode addr starts : %p\n", NVM_METADATA_START);
	printf("data block starts : %p\n", NVM_DATA_START);
	printf("------------------[END]-----------------------\n");
}

VT_entry* search_vt_entry(unsigned int vid)
{
	int i;
	VT_entry* veptr = NVM_VOL_TABLE_START;

	for(i = 0; i < MAX_VT_ENTRY; i++) {
		if(veptr->vid == vid)
			return veptr;
		veptr++;
	}
	
	return NULL;
}

VT_entry* get_vt_entry(unsigned int vid)
{
	VT_entry* veptr;


	while((veptr = search_vt_entry(0)) == NULL)
		;

	veptr->vid = vid;

//	while(1)
//	{
//		if(__sync_bool_compare_and_swap(&veptr->vid, 0, vid)
//				break;
//		veptr++;
//	}



	return veptr;
}

NVM_metadata* get_nvm_metadata()
{
	NVM_metadata* mdptr =
		__sync_lock_test_and_set(&NVM_FREE_LIST_HEAD, NVM_METADATA_START + NVM_FREE_LIST_HEAD->next);
	
	mdptr->next = 0;
	mdptr->flag = FLAG_ALLOCATED;
	
	return mdptr;
}

void nvm_atomic_write(unsigned int vid, unsigned int ofs, void* ptr, unsigned int len)
{
	// Search VT_entry of vid
	VT_entry* ve = search_vt_entry(vid);

	if(ve == NULL) 
	{
		// Allocate new VT_entry and Metadata
		ve = get_vt_entry(vid);
		NVM_metadata* new_md = get_nvm_metadata();
		ve->vhead = (unsigned int)(new_md - NVM_METADATA_START);
	}

	NVM_metadata* mdptr = NVM_METADATA_START + ve->vhead;

	unsigned int write_bytes = 0;
	unsigned int lbn = ofs / BLOCK_SIZE;
	ofs = ofs % BLOCK_SIZE;

	// Access to lbn representing metadata
	while(lbn > 0 && mdptr->next != 0) 
	{
		mdptr = NVM_METADATA_START + mdptr->next;
		lbn--;
	}

	// write data to nvm
	unsigned int block_idx;
	char* data_dst;
	
	block_idx = (unsigned int)(mdptr - NVM_METADATA_START);
	data_dst = NVM_DATA_START + block_idx * BLOCK_SIZE + ofs;
	write_bytes = (len > BLOCK_SIZE - ofs) ? (BLOCK_SIZE - ofs) : len;
	memcpy(data_dst, ptr, write_bytes);
	mdptr->flag = FLAG_WRITTEN; // flag changed to WRITTEN
	len -= write_bytes;

	while(len > 0) {
		// allocate more block
		NVM_metadata* new_md = get_nvm_metadata();
		block_idx = (unsigned int)(new_md - NVM_METADATA_START);
		mdptr->next = block_idx;
		mdptr = NVM_METADATA_START + mdptr->next;
		data_dst = NVM_DATA_START + block_idx * BLOCK_SIZE + ofs;
		write_bytes = (len > BLOCK_SIZE - ofs) ? (BLOCK_SIZE - ofs) : len;
		memcpy(data_dst, ptr, write_bytes);
		mdptr->flag = FLAG_WRITTEN;
		len -= write_bytes;
	}

}

#endif
