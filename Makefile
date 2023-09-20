CC=clang
CFLAGS=-Wall -Wextra -Werror -Wpedantic

all: encode decode

encode: encode.c trie.c word.c io.c
	$(CC) $(CFLAGS) $^ -o $@  -lm -lgmp -lcheck -lpthread -lsubunit

decode: decode.c trie.c word.c io.c
	$(CC) $(CFLAGS) $^ -o $@ -lm -lgmp -lcheck -lpthread -lsubunit \

format:
	clang-format -i -style=file *.[ch]

clean:
	rm -f encode decode *.o

