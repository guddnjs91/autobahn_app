#ifndef nvm0prototypes_h
#define nvm0prototypes_h

/* in file nvm0nvm.cc */
void nvm_structure_build();
void print_nvm_info();
void nvm_system_init();
void nvm_system_close();

/* in file nvm0avltree.cc */
tree_node* search_tree_node(tree_root* tree, uint32_t lbn);
void insert_tree_node(tree_root* tree, tree_node* node);
tree_node* insert_tree_node(tree_node* root, tree_node* node);
tree_node* physical_delete_tree_node(tree_node* root, tree_node* node);
void logical_delete_tree_node(tree_root* tree, tree_node* node);
void rebalance_tree_node(tree_root* tree);
tree_node* find_invalid_tree_node(tree_node* node);

tree_node* init_tree_node(inode_entry* inode);
tree_node* min_value_node(tree_node* node);
int max_height(int a, int b);
int get_height(tree_node* node);
int get_balance(tree_node* node);
tree_node* right_rotate(tree_node* y);
tree_node* left_rotate(tree_node* x);
double get_invalid_ratio(tree_root *tree);

/* in file nvm0write.c */
volume_idx_t get_volume_entry_idx(uint32_t vid);
volume_idx_t search_volume_entry_idx(uint32_t vid);
volume_idx_t alloc_volume_entry_idx(uint32_t vid);
const char* get_filename(uint32_t vid);
inode_idx_t get_inode_entry_idx(volume_entry* ve, uint32_t lbn);
inode_idx_t alloc_inode_entry_idx(uint32_t lbn);

size_t nvm_write(uint32_t vid, off_t ofs, const void* ptr, size_t len);
//void nvm_atomic_write(unsigned int vid, unsigned int ofs, void* ptr, unsigned int len);

/* in file nvm0flush.c */
void* flush_thread_func(void* data);
void nvm_flush(void);

/* in file nvm0balloon.c */
void* balloon_thread_func(void* data);
void nvm_balloon(void);

#endif
