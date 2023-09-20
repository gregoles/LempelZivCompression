# Lempel Ziv Compression

## Short Description:
This program contains two programs encode and decode as well as three other helper programs. These programs carry out LZ78 compression and decompression, respectively. encode.c and decode.c had to be able compress any type of file, whether it was text or binary. \
The three helper programs are called trie.c, word.c, and io.c. These programs defined functions for trie root, word, and io operations that could be used in the encode and decode programs.

## Build
To build the executables, run '$ make' or '$ make all' on the command line within the assignment directory. This will could create executable binary files titled 'encode' and 'decode', which the user can then run.

## Cleaning
To clean the directory, type '$ make clean' into the assignment directory. This will delete the executable files as well as the .o files.
