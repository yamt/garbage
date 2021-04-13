run-test: test_alt_entry test_normal
	./test_normal
	./test_alt_entry

.c.o:
	$(CC) -c -o $@ $<

test_alt_entry: a.o
	$(CC) -nostartfiles -e _entry -o $@ $<

test_normal: a.o
	$(CC) -o $@ $<

clean:
	rm -f a.o
	rm -f test_alt_entry
	rm -f test_normal
