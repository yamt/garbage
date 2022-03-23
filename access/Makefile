
test: access
	rm -rf testfile
	./access testfile
	touch testfile
	chmod 777 testfile
	./access testfile
	chmod 000 testfile
	./access testfile

access: access.c
	cc -o $@ $<

clean:
	rm -rf testfile
	rm -rf access
