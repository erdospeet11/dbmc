#include "b_tree.h"

int main() {
    printf("B-Tree Implementation Test\n");
    printf("==========================\n\n");
    
    BTreeNode* root = NULL;
    
    int keys[] = {10, 20, 5, 6, 12, 30, 7, 17, 1, 3, 8, 25, 35, 40};
    int num_keys = sizeof(keys) / sizeof(keys[0]);
    
    printf("1. Testing insertion of keys: ");
    for (int i = 0; i < num_keys; i++) {
        printf("%d ", keys[i]);
    }
    printf("\n\n");
    
    for (int i = 0; i < num_keys; i++) {
        root = insert(root, keys[i]);
        if (root == NULL) {
            printf("Error: Failed to insert key %d\n", keys[i]);
            return 1;
        }
        printf("Inserted %d\n", keys[i]);
    }
    
    printf("\n2. Tree traversal (in-order):\n");
    if (is_empty(root)) {
        printf("Tree is empty\n");
    } else {
        traverse(root);
        printf("\n");
    }
    
    printf("\n3. Testing search functionality:\n");
    int search_keys[] = {7, 15, 25, 50};
    int num_search = sizeof(search_keys) / sizeof(search_keys[0]);
    
    for (int i = 0; i < num_search; i++) {
        BTreeNode* result = search(root, search_keys[i]);
        if (result != NULL) {
            printf("Key %d: FOUND\n", search_keys[i]);
        } else {
            printf("Key %d: NOT FOUND\n", search_keys[i]);
        }
    }
    
    printf("\n4. Testing delete functionality:\n");
    delete_key(root, 10);
    delete_key(root, 25);
    
    printf("\n5. Final tree traversal:\n");
    traverse(root);
    printf("\n");
    
    printf("\n6. Cleaning up memory...\n");
    free_tree(root);
    printf("Memory cleanup completed.\n");
    
    printf("\nB-Tree test completed successfully!\n");
    
    return 0;
} 