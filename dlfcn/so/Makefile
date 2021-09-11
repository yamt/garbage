CC = clang
LD = ld

CFLAGS = -Os
CFLAGS += -Wall -Werror
CFLAGS += -fPIC

OBJS = test.o
BIN = test.so

# XXX soname

$(BIN): $(OBJS)
	$(CC) \
	-shared \
	-Wl,-undefined,dynamic_lookup \
	-o $@ \
	$<

.c.o:
	$(CC) \
	-c \
	$(CFLAGS) \
	-o $@ \
	$<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
