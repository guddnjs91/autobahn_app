/* in file nvm0nvm.c */
void nvm_structure_build();
void print_nvm_info();
void nvm_system_init();
void nvm_system_close();

/* in file nvm0write.c */
void nvm_write(uint32_t vid, off_t ofs, const void* ptr, size_t len);

//void nvm_atomic_write(unsigned int vid, unsigned int ofs, void* ptr, unsigned int len);
volume_entry* get_volume_entry(uint32_t vid);
volume_entry* search_volume_entry(volume_entry* vt_base, uint32_t vid);
volume_entry* alloc_volume_entry(uint32_t vid);
inode_entry* get_inode_entry(volume_entry* ve, uint32_t lbn);
inode_entry* alloc_inode_entry(uint32_t lbn);
const char* get_filename(uint32_t vid);

/* in file nvm0flush.c */
void* flush_thread_func(void* data);
void* balloon_thread_func(void* data);
//void nvm_flush(void);

/* in file nvm0avltree.c */
//NVM_inode* search_nvm_inode(NVM_inode* root, unsigned int lbn);
//NVM_inode* insert_nvm_inode(NVM_inode* root, NVM_inode* inode);
//int Max(int a, int b);
//int height(NVM_inode* N);
//int getBalance(NVM_inode* N);
//NVM_inode* rightRotate(NVM_inode* y);
//NVM_inode* leftRotate(NVM_inode* y);
//NVM_inode* delete_nvm_inode(NVM_node* root, NVM_inode* inode);
//NVM_inode* min_value_node(NVM_inode* inode);
//void deallocate_node(NVM_inode* inode);
