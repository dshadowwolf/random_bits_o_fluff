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
  node_t* left;
  node_t* right;
};

typedef struct table_s table_t;

struct table_s {
  char *id;
  uint32_t hash;
  node_t* buckets[256];
};

static table_t tables[256];  // we don't need more than 256 tables for language feature stuff

node_t* make_node(char* name) {
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

table_t get_table(char *name) {
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
        printf("Key with name %s already exists!\n", insert->val.key);
        exit(-1);
      } else {
        /*
         * TODO: If two keys hash exactly the same, there will be a direct collision. This can probably be resolved by doing a sub-insert but...
         * That feels rather messy.
         */
        printf("Hash Match, Key Mismatch: TODO -- Fix This Issue (%s -- %s)\n", name, work->val.key);
        exit(-1);
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

node_t* add_to_table(char *table, char* key) {
  table_t t = get_table(table);

  if (t.id == NULL || t.hash == 0) {
    printf("Asked for table %s, no such table exists, creating\n", table);
    add_table(table);
    t = get_table(table);
  }

  node_t* nn = make_node(key);
  insert_node(name, nn, &t);
  return nn;
}

node_t* get_node(char *name, node_t* bucket) {
  node_t* work = bucket;
  uint32_t cc = crc32(name);
  while(1) {
    if (strcmp(bucket->val.key, name) == 0) {
      return bucket;
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
