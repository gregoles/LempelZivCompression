#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "word.h"
#include "code.h"

Word *word_create(uint8_t *syms, uint32_t len) {
    Word *w = (Word *) malloc(sizeof(Word));
    if (w == NULL) {
        return NULL;
    }
    w->syms = (uint8_t *) malloc(len * sizeof(uint8_t));
    if (w->syms == NULL) {
        free(w);
        return NULL;
    }
    for (uint32_t i = 0; i < len; i++) {
        w->syms[i] = syms[i];
    }
    w->len = len;
    return w;
    //allocate memory for a word pointer
    //set its fields
    //for each stm in syms set sym in word pointer
    //return your word pointer
}

Word *word_append_sym(Word *w, uint8_t sym) {
    uint32_t new_len = w->len + 1;
    uint8_t *new_syms = (uint8_t *) realloc(w->syms, new_len * sizeof(uint8_t));
    if (new_syms == NULL) {
        return NULL;
    }

    new_syms[new_len - 1] = sym;
    ////Word *new_w = (Word *)malloc(sizeof(Word));
    Word *new_w = word_create(new_syms, new_len);
    if (new_w == NULL) {
        free(new_syms);
        return NULL;
    }
    return new_w;

    //call word_create, but with w len + 1
    //set character at address w len to sym
    //return new word
}

void word_delete(Word *w) {
    free(w->syms);
    free(w);
}

WordTable *wt_create(void) {
    WordTable *wt = (WordTable *) malloc(MAX_CODE * sizeof(Word *));
    if (wt == NULL) {
        return NULL;
    }
    wt[EMPTY_CODE] = word_create(NULL, 0);
    if (wt[EMPTY_CODE] == NULL) {
        free(wt);
        return NULL;
    }
    for (int i = 0; i < MAX_CODE; i++) {
        if (i != EMPTY_CODE) {
            wt[i] = NULL;
        }
    }
    return wt;

    //WordTable* == Word**
}

void wt_reset(WordTable *wt) {
    //printf("hello");
    for (int i = 0; i < MAX_CODE; i++) {
        if (i == EMPTY_CODE) {
            continue;
        }
        if (wt[i] != NULL) {
            //word_delete(wt[i]);
            wt[i] = NULL;
            //word_delete(wt[i]);
        }
        //wt[i] = NULL;
    }
}

void wt_delete(WordTable *wt) {
    for (int i = 0; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            word_delete(wt[i]);
        }
    }
    free(wt);

    // for each index
}
