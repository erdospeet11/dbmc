#ifndef B_TREE_H
#define B_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct BTreeNode {
    int is_leaf;
    int num_keys;
    int* keys;
    struct BTreeNode** children;
} BTreeNode;

BTreeNode* create_node(int is_leaf);
BTreeNode* search(BTreeNode* node, int key);
BTreeNode* insert(BTreeNode* root, int key);
void insert_key(BTreeNode* node, int key);
void split_child(BTreeNode* parent, int index);
void delete_key(BTreeNode* root, int key);
void traverse(BTreeNode* node);
void free_tree(BTreeNode* node);
int is_empty(BTreeNode* node);

#endif