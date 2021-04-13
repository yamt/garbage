all: test

.c.o:
	$(CC) -c -o $@ $<

test: a.o
	$(CC) -nostartfiles -e entry -o $@ $<

clean:
	rm -f a.o
	rm -f test
