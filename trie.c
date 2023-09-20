#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "trie.h"
#include "code.h"

TrieNode *trie_node_create(uint16_t index) {
    TrieNode *tn = (TrieNode *) malloc(sizeof(TrieNode));
    for (int i = 0; i < ALPHABET; i++) {
        tn->children[i] = NULL;
    }
    tn->code = index;
    return tn;
}

void trie_node_delete(TrieNode *n) {
    //call free on each of n's children
    //for child in n -> free
    //free n
    if (n == NULL) {
        return;
    }
    for (int i = 0; i < ALPHABET; i++) {
        trie_node_delete(n->children[i]);
    }
    free(n);
}

TrieNode *trie_create(void) {
    TrieNode *root = trie_node_create(EMPTY_CODE);
    if (root == NULL) {
        return NULL;
    }
    return root;

    //call trie_node_create with code set to EMPTY_CODE
    //that will be your root node
}

void trie_reset(TrieNode *root) {
    for (int i = 0; i < ALPHABET; i++) {
        if (root->children[i] != NULL) {
            trie_delete(root->children[i]);
            root->children[i] = NULL;
        }
    }
}

void trie_delete(TrieNode *n) {
    if (n == NULL) {
        return;
    }
    for (int i = 0; i < ALPHABET; i++) {
        trie_delete(n->children[i]);
        n->children[i] = NULL;
    }
    trie_node_delete(n);
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    return n->children[sym];
}
