#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "word.h"
#include "io.h"
#include "code.h"
#include <math.h>
#include <sys/stat.h>

#define DEFAULT_INPUT  "-"
#define DEFAULT_OUTPUT "-"

int bit_length(uint16_t num) {
    return num == 0 ? 1 : (int) log2(num) + 1;
}

int main(int argc, char *argv[]) {
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    char *input_file = DEFAULT_INPUT;
    char *output_file = DEFAULT_OUTPUT;
    int opt;

    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
        case 'i': input_file = optarg; break;
        case 'o': output_file = optarg; break;
        default: fprintf(stderr, "Usage: %s [-i input] [-o output]\n", argv[0]); exit(EXIT_FAILURE);
        }
    }

    if (strcmp(input_file, "-") != 0) {
        infile = open(input_file, O_RDONLY);
        if (infile < 0) {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
    }

    if (strcmp(output_file, "-") != 0) {
        outfile = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (outfile < 0) {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
    }

    FileHeader header;
    //header.magic = 0xBAADBAAC;
    //header.protection = statbuf.st_mode;

    uint16_t curr_code;
    uint8_t curr_sym;
    WordTable *table = wt_create();
    if (table == NULL) {
        fprintf(stderr, "Error creating word table\n");
        exit(EXIT_FAILURE);
    }

    read_header(infile, &header);

    curr_sym = 0;
    curr_code = 0;
    uint16_t next_code = START_CODE;

    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code)) == true) {
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile, table[next_code]);
        next_code = next_code + 1;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile);
    //wt_delete(table);
    close(infile);
    close(outfile);
}
