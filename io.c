#include "endian.h"
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include "code.h"

#include "io.h"

#define BLOCK_SIZE  4096
#define BUFFER_SIZE 4096

static uint8_t sym_buffer[BUFFER_SIZE];
static uint8_t pair_buffer[BUFFER_SIZE];
static int sym_index = BUFFER_SIZE;
static int pair_index = 0; //BUFFER_SIZE * 8;

int read_bytes(int infile, uint8_t *buf, int to_read) {
    //printf("testing");
    int bytes_read = 0;
    int temp = 0;
    while (bytes_read < to_read && (temp = read(infile, buf, to_read - bytes_read))) {
        // ||
        bytes_read += temp;
    }
    return bytes_read;
}

int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int bytes_wrote = 0;
    int temp = 0;
    while (bytes_wrote < to_write && (temp = write(outfile, buf, to_write - bytes_wrote))) {
        bytes_wrote += temp;
    }
    return bytes_wrote;
}

void read_header(int infile, FileHeader *header) {
    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));
    if (header->magic != MAGIC) {
        fprintf(stderr, "Error: Invalid magic number\n");
        exit(1);
    }
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
}

void write_header(int outfile, FileHeader *header) {
    if (big_endian()) {
        uint32_t swapped_magic = swap32(header->magic);
        uint16_t swapped_protection = swap16(header->protection);
        write_bytes(outfile, (uint8_t *) &swapped_magic, sizeof(header->magic));
        write_bytes(outfile, (uint8_t *) &swapped_protection, sizeof(header->protection));
    } else {
        write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
    }
}

// call read_bytes() function with to_read() set to sizeof(FileHeader)
// create a buffer sizeof(FileHeader)
// and to_read is also sizeof(FileHeader)

bool read_sym(int infile, uint8_t *sym) {
    //printf(sym);
    if (sym_index == BUFFER_SIZE) {
        int bytes_read = read_bytes(infile, sym_buffer, BUFFER_SIZE);
        if (bytes_read == 0) {
            return false;
        }
        sym_index = 0;
    }
    //printf(sym);
    *sym = sym_buffer[sym_index++];
    //printf(sym);
    return true;
}

void flush_buffer(int outfile) {
    if (pair_index > 0) {
        int num_bytes = (pair_index + 7) / 8;
        write_bytes(outfile, pair_buffer, num_bytes);
        pair_index = 0;
    }
}

void flush_pairs(int outfile) {
    if (pair_index % 8 != 0) {
        pair_buffer[pair_index / 8] |= (1 << (pair_index % 8)) - 1;
        pair_index += 8 - (pair_index % 8);
    }
    write_bytes(outfile, pair_buffer, pair_index / 8);
    pair_index = BUFFER_SIZE * 8;
}

void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    ////if(big_endian()){
    ////code = swap16(code);
    uint32_t value = (sym << 8) | code;
    int total_bits = bitlen + 8;
    for (int i = 0; i < total_bits; i++) { //total_bits
        int bit = (value >> i) & 1;
        pair_buffer[pair_index >> 3] |= bit << (pair_index & 7);
        pair_index++;
        if (pair_index == BUFFER_SIZE * 8) {
            //flush_pairs(outfile);
            write_bytes(outfile, pair_buffer, BUFFER_SIZE);
            printf("%s", pair_buffer);
        }
    }
}

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    static int num_bits = 0;
    static int num_pairs = 0;
    static int buffer_pos = 0;
    while (true) {
        if (pair_index >= num_bits) {
            num_pairs = read_bytes(infile, pair_buffer, BUFFER_SIZE);
            if (num_pairs == 0) {
                return false;
            }
            num_bits = num_pairs * 8;
            buffer_pos = 0;
        }
        if (num_bits - pair_index < bitlen + 8) {
            return false;
        }
        uint32_t value = 0;
        for (int i = 0; i < bitlen; i++) { // + 8
            int bit = (pair_buffer[buffer_pos + (pair_index >> 3)] >> (pair_index & 7)) & 1;
            value |= bit << i;
            pair_index++;
        }
        buffer_pos += (pair_index >> 3);
        pair_index &= 7;
        *code = value;
        *sym = pair_buffer[buffer_pos + (pair_index >> 3)];
        pair_index += 8;
        //*code = (value >> 8);
        //*sym = (value & 0xFF);
        if (*code == 0) {
            return false;
        }
        //buffer_pos += (pair_index >> 3);
        //pair_index &= 7;
        return true;
    }
}

void write_word(int outfile, Word *w) {
    for (uint32_t i = 0; i < w->len; i++) {
        if (sym_index == BUFFER_SIZE) {
            write_bytes(outfile, sym_buffer, sym_buffer[sym_index]);
            sym_index = 0;
        }
        sym_buffer[sym_index++] = w->syms[i];
    }

    if (sym_index > 0) {
        write_bytes(outfile, sym_buffer, sym_index);
        sym_index = 0;
    }
}

void flush_words(int outfile) {
    if (sym_index > 0) {
        write_bytes(outfile, sym_buffer, sym_index);
        sym_index = BUFFER_SIZE;
    }
    struct stat statbuf;
    fstat(outfile, &statbuf);
    fchmod(outfile, statbuf.st_mode);
}
