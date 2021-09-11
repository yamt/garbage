CC = cc
LD = ld

UNAME = $(shell uname -s)
CFLAGS = -Os
CFLAGS += -Wall -Werror
CFLAGS += -fPIC
CLINKFLAGS = -shared

ifeq ($(UNAME),Darwin)
CLINKFLAGS += -Wl,-undefined,dynamic_lookup
endif

OBJS = test.o
BIN = test.so

# XXX soname

$(BIN): $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
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
