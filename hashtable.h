typedef struct node_s node_t; // forward declaration for the binary tree code
typedef struct list_s list_t; // forward declaration for fully colliding hashes to be resolved

void *node_get(char *table, char *name);
void node_set(char *table, char *name, void* data);
void register_node(char *table, char* key);
void add_table(char *name);


