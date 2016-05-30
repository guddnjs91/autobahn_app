
/* in file nvm0nvm.c */
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

/* in file nvm0write.c */
/* in file nvm0flush.c */
