/* in file nvm0nvm.c */



/* in file nvm0write.c */
//void nvm_atomic_write(unsigned int vid, unsigned int ofs, void* ptr, unsigned int len);
//VT_entry* get_vt_entry(unsigned int vid);
//VT_entry* search_vt_entry(VT_entry* vt_root, unsigned int vid);
//VT_entry* alloc_vt_entry(unsigned int vid);
//NVM_inode* get_nvm_inode(VT_entry* vte, unsigned int lbn);
//NVM_inode* alloc_nvm_inode(unsigned int lbn);
//const char* get_filename(unsigned int vid);

/* in file nvm0flush.c */
//void* flush_thread_func(void* data);
//void nvm_flush(void);
//void* reclaim_thread_func(void* data);

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
