all: test

.c.o:
	$(CC) -c -o $@ $<

test: a.o
	$(CC) -nostartfiles -e entry -o $@ $<
