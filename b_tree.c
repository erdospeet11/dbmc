#include "b_tree.h"

#define T 2

BTreeNode* create_node(int is_leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for BTreeNode\n");
        return NULL;
    }
    
    node->keys = (int*)malloc(sizeof(int) * (2 * T - 1));
    if (!node->keys) {
        fprintf(stderr, "Error: Memory allocation failed for keys\n");
        free(node);
        return NULL;
    }
    
    node->children = (BTreeNode**)malloc(sizeof(BTreeNode*) * 2 * T);
    if (!node->children) {
        fprintf(stderr, "Error: Memory allocation failed for children\n");
        free(node->keys);
        free(node);
        return NULL;
    }
    
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    
    //init children to NULL
    for (int i = 0; i < 2 * T; i++) {
        node->children[i] = NULL;
    }
    
    return node;
}

void insert_key(BTreeNode* node, int key) {
    int i = node->num_keys - 1;
    if (node->is_leaf) {
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->num_keys++;
    } else {
        while (i >= 0 && node->keys[i] > key) {
            i--;
        }
        i++;
        if (node->children[i]->num_keys == 2 * T - 1) {
            split_child(node, i);
            if (node->keys[i] < key) {
                i++;
            }
        }
        insert_key(node->children[i], key);
    }
}

void split_child(BTreeNode* parent, int index) {
    BTreeNode* child = parent->children[index];
    BTreeNode* new_child = create_node(child->is_leaf);
    new_child->num_keys = T - 1;
    for (int i = 0; i < T - 1; i++) {
        new_child->keys[i] = child->keys[i + T];
    }
    if (!child->is_leaf) {
        for (int i = 0; i < T; i++) {
            new_child->children[i] = child->children[i + T];
        }
    }
    child->num_keys = T - 1;
    for (int i = parent->num_keys; i >= index + 1; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = new_child;
    for (int i = parent->num_keys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }
    parent->keys[index] = child->keys[T - 1];
    parent->num_keys++;
}

void traverse(BTreeNode* node) {
    if (node == NULL) return;
    
    int i;
    for (i = 0; i < node->num_keys; i++) {
        if (!node->is_leaf) {
            traverse(node->children[i]);
        }
        printf("%d ", node->keys[i]);
    }
    if (!node->is_leaf) {
        traverse(node->children[i]);
    }
}

BTreeNode* search(BTreeNode* node, int key) {
    if (node == NULL) return NULL;
    
    int i = 0;
    while (i < node->num_keys && key > node->keys[i]) {
        i++;
    }
    
    if (i < node->num_keys && key == node->keys[i]) {
        return node;
    }
    
    if (node->is_leaf) {
        return NULL;
    }
    
    return search(node->children[i], key);
}

BTreeNode* insert(BTreeNode* root, int key) {
    if (root == NULL) {
        root = create_node(1);
        if (root == NULL) return NULL;
        root->keys[0] = key;
        root->num_keys = 1;
        return root;
    }
    
    if (root->num_keys == 2 * T - 1) {
        BTreeNode* new_root = create_node(0);
        if (new_root == NULL) return root;
        
        new_root->children[0] = root;
        split_child(new_root, 0);
        
        int i = 0;
        if (new_root->keys[0] < key) {
            i++;
        }
        insert_key(new_root->children[i], key);
        
        return new_root;
    } else {
        insert_key(root, key);
        return root;
    }
}

void delete_key(BTreeNode* root, int key) {
    //simple delete for now
    //TODO: implement full delete with merging and borrowing
    printf("Delete operation not fully implemented yet for key: %d\n", key);
}

void free_tree(BTreeNode* node) {
    if (node == NULL) return;
    
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            free_tree(node->children[i]);
        }
    }
    
    free(node->keys);
    free(node->children);
    free(node);
}

int is_empty(BTreeNode* node) {
    return (node == NULL || node->num_keys == 0);
}
