BIN = listenexec
OBJS = main.o
CFLAGS = -Os
CC = cc

$(BIN): $(OBJS)
	$(CC) \
	-o $@ \
	$(OBJS) \
	$(LIBS)

.c.o:
	$(CC) -c \
	$(CFLAGS) \
	-o $@ \
	-g \
	$<

clean:
	rm -f $(BIN)
	rm -f $(OBJS)

style:
	clang-format-12 -i *.[ch]
