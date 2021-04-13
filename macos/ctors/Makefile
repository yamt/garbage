all: test

.c.o:
	$(CC) -c -o $@ $<

test: a.o
	$(CC) -e _entry -o $@ $<
