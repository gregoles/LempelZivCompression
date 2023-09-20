#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "trie.h"
#include "code.h"
#include "io.h"
#include "string.h"
#include <getopt.h>

#include <math.h>

int bit_length(uint16_t num) {
    return num == 0 ? 1 : (int) log2(num) + 1;
}

int main(int argc, char *argv[]) {
    struct stat statbuf;
    char *input_file = NULL;
    char *output_file = NULL;
    int opt;
    int infile, outfile;

    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
        case 'i': input_file = optarg; break;
        case 'o': output_file = optarg; break;
        default: fprintf(stderr, "Usage: %s [-i input] [-o output]\n", argv[0]); exit(1);
        }
    }

    // Open input file or set it to stdin if no input file specified
    if (input_file == NULL) {
        infile = STDIN_FILENO;
    } else {
        infile = open(input_file, O_RDONLY);
        if (infile < 0) {
            perror("Error opening input file");
            exit(1);
        }
    }

    // Read input file size and protection bits
    if (fstat(infile, &statbuf) < 0) {
        perror("Error getting input file stats");
        exit(1);
    }

    if (output_file == NULL) {
        outfile = STDOUT_FILENO;
    } else {
        outfile = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, statbuf.st_mode);
        if (outfile < 0) {
            perror("Error opening output file");
            exit(1);
        }
    }

    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    uint16_t next_code = START_CODE;

    FileHeader header;
    memset(&header, 0, sizeof(FileHeader));
    header.magic = 0xBAADBAAC;
    header.protection = statbuf.st_mode;

    write_header(outfile, &header);

    while (read_sym(infile, &curr_sym) == true) {
        TrieNode *next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1;
        }
        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
    flush_pairs(outfile);
    trie_delete(root);

    close(infile);
    close(outfile);
}
