#include <stdlib.h>    // malloc, et. al.
#include <stdio.h>     // printf and friends
#include <string.h>    // strcmp/strcpy/etc...
#include <inttypes.h>  // uint32_t and friends

#include "hashtable.h"
#include "crc32.h"

struct node_s {
  struct {
    char* key;
    uint32_t code;
  } val;

  void* data;
  int8_t is_volatile;
  list_t *collisions;
  node_t* left;
  node_t* right;
};

typedef struct table_s table_t;
typedef struct list_s list_t;

struct list_s {
  char *id;
  void *data;
  list_t *next;
};

struct table_s {
  char *id;
  uint32_t hash;
  node_t* buckets[256];
};

static table_t tables[256];  // we don't need more than 256 tables for language feature stuff

static node_t* make_node(char* name) {
  node_t* rval = malloc(sizeof(node_t));
  rval->val.code = crc32(name);
  rval->val.key = name;
  return rval;
}

void add_table(char *name) {
  uint32_t table_hash = crc32(name);
  uint8_t table_bucket = table_hash % 256;
  if (tables[table_bucket].id != NULL && strcmp(tables[table_bucket].id, name) == 0) {
    printf("Table with name %s already registered\n", name);
  } else if(tables[table_bucket].id != NULL) {
    printf("Bucket Collision: Table %s has the same bucket as %s\n", tables[table_bucket].id, name);
  } else {
    tables[table_bucket].id = name;
    tables[table_bucket].hash = table_hash;
  }
}

static table_t get_table(char *name) {
  uint32_t table_hash = crc32(name);
  uint8_t table_bucket = table_hash % 256;
  return tables[table_bucket];
}

static void __insert_inner(char *name, node_t* insert, node_t* tree) {
  uint32_t curr = insert->val.code;
  uint32_t base = insert->val.code;
  node_t* work = tree;

  for (int k = 0; k < 32; k++) {
    if (work->val.code == base) {
      if (strcmp(work->val.key, insert->val.key) == 0) {
        // overwrite existing keys
        work->data = insert->data;
        free(insert);
        return;      } else {
        // if we have a hash collision, insert into a linked list
        list_t* nn = malloc(sizeof(list_t));
        nn->id = insert->val.key;
        nn->data = insert->data;
        free(insert);

        if (work->collisions == NULL) work->collisions = nn;
        else {
          list_t* wl = work->collisions;
          while (wl->next != NULL) wl = wl->next;
          wl->next = nn;
        }
      }
    } else {
      if (curr & 1) {
        if (work->left != NULL) {
          work = work->left;
        } else {
          work->left = insert;
          return;
        }
      } else {
        if (work->right != NULL) {
          work = work->right;
        } else {
          work->right = insert;
          return;
        }
      }
    }
    curr >>= 1;
  }

  printf("Could not find a place for \"%s\" after checking all 32 bits!\n", name);
  exit(-1);
}

static void insert_node(char *name, node_t* node, table_t* table) {
  uint8_t bid = node->val.code % 256;
  if (table->buckets[bid] == NULL) {
    table->buckets[bid] = node;
  } else {
    __insert_inner(name, node, table->buckets[bid]);
  }
}

void register_node(char *table, char* key) {
  table_t t = get_table(table);

  if (t.id == NULL || t.hash == 0) {
    printf("Asked for table %s, no such table exists, creating\n", table);
    add_table(table);
    t = get_table(table);
  }

  node_t* nn = make_node(key);
  insert_node(name, nn, &t);
}

static node_t* __get_internal(char *name, node_t* bucket) {
  node_t* work = bucket;
  uint32_t cc = crc32(name);
  while(1) {
    if (strcmp(bucket->val.key, name) == 0) {
      return bucket;
    } else if (bucket->val.code == cc) {
      list_t* ll = bucket->collisions;
      while(ll != NULL) {
        if (strcmp(ll->id, name) == 0) {
          node_t* rr = make_node(name);
          rr->data = ll->data;
          rr->is_volatile = 1;
          return rr;
        }
        ll = ll->next;
      }
      printf("Entry with id \"%s\" not found in data!\n", name);
      return NULL;
    }

    if (cc & 1) {
      if (work->left != NULL) work = work->left;
      else {
        printf("Key \"%s\" not found in bucket!", name);
        return NULL;
      }
    } else {
      if (work->right != NULL) work = work->right;
      else {
        printf("Key \"%s\" not found in bucket!", name);
        return NULL;
      }
    }
    cc >>= 1;
  }
}

static void* get_node(char *name, node_t* bucket) {
  node_t* base = __get_internal(name, bucket);
  if (base == NULL) {
    return NULL;
  }

  void *rv = base->data;
  if (base->is_volatile) free(base);
  return rv;
}

static void set_node_data(char *name, node_t* bucket, void* data) {
  node_t* base = __get_internal(name, bucket);

  if (base == NULL) {
    base = make_node(name);
  }
  base->data = data;
  __insert_inner(name, base, bucket);
}

void *node_get(char *table, char *name) {
  table_t* source = get_table(table);
  uint32_t node_crc = crc32(name);
  uint8_t bid = node_crc % 256;

  return get_node(name, source->buckets[bid]);
}

void node_set(char *table, char *name, void* data) {
  table_t* source = get_table(table);
  uint32_t node_crc = crc32(name);
  uint8_t bid = node_crc % 256;
  set_node_data(name, source->buckets[bid], data);
}
