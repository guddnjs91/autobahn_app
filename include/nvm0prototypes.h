#ifndef nvm0prototypes_h
#define nvm0prototypes_h

/* in file nvm0nvm.cc */
void nvm_structure_build();
void nvm_system_init();
void nvm_system_close();
void nvm_structure_destroy();
void print_nvm_info();

/* in file nvm0write.cc */
size_t nvm_durable_write(uint32_t vid, off_t ofs, const char* ptr, size_t len);

/* in file nvm0volume.cc */
volume_idx_t get_volume_entry_idx(uint32_t vid);
volume_idx_t search_volume_entry_idx(uint32_t vid);
volume_idx_t alloc_volume_entry_idx(uint32_t vid);

/* in file nvm0flush.cc */
void* flush_thread_func(void* data);

/* in file nvm0balloon.cc */
void* balloon_thread_func(void* data);

/* in file nvm0sync.cc */
void* sync_thread_func(void* data);

/* in file nvm0hash.cc */
struct hash_table* new_hash_table();
void validate_hash_node(struct hash_node* hash_node, struct inode_entry* inode);
struct hash_node* new_hash_node(inode_entry* inode);
void insert_hash_node(struct hash_table *table, hash_node *node);
struct hash_node* search_hash_node(struct hash_table *table, uint32_t lbn);
void logical_delete_hash_node(struct hash_table *table, hash_node *node);
void physical_delete_hash_node(struct hash_table *table, hash_node *node);

/* in file nvm0list.cc */
struct list* new_list();
void push_back_list_node(struct list* list, struct hash_node* node);
struct hash_node* pop_front_list_node(struct list* list);
void remove_list_node(struct list* list, struct hash_node* node);

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

#endif
