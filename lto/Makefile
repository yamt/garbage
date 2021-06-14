BIN = foo
OBJS = a.o f.o

CC = cc
LD = ld -arch x86_64 -macos_version_min 10.14

test: $(BIN)
	objdump -r -d $(BIN)
	nm $(BIN)

$(BIN): $(OBJS)
	$(LD) \
	-o $@ \
	$(OBJS) \
	-lc

.c.o:
	$(CC) -c \
	-o $@ \
	-flto \
	-Os \
	-g \
	$<

clean:
	rm -f $(BIN)
	rm -f $(OBJS)
