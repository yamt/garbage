all: test

.c.o:
	$(CC) -c -o $@ $<

test: a.o
	$(CC) -nostartfiles -e _entry -o $@ $<

clean:
	rm -f a.o
	rm -f test
