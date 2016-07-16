/* in file nvm0nvm.c */
void nvm_structure_build();
void nvm_system_init();
void nvm_system_close();
void nvm_structure_destroy();

/* in file nvm0write.c */
void nvm_write(uint32_t vid, off_t ofs, const void* ptr, size_t len);

/* in file nvm0flush.c */
void* flush_thread_func(void* data);
void* balloon_thread_func(void* data);

/* in file nvm0avltree.c */
//NVM_inode* search_nvm_inode(NVM_inode* root, unsigned int lbn);
//NVM_inode* insert_nvm_inode(NVM_inode* root, NVM_inode* inode);
//int Max(int a, int b);
//int height(NVM_inode* N);
//int getBalance(NVM_inode* N);
//NVM_inode* delete_nvm_inode(NVM_node* root, NVM_inode* inode);
//NVM_inode* min_value_node(NVM_inode* inode);
//void deallocate_node(NVM_inode* inode);
