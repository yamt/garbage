UNAME = $(shell uname -s)

BIN = foo
OBJS = a.o f.o

CC = cc
ifeq ($(UNAME),Darwin)
LD = ld -arch x86_64 -macos_version_min 10.14
LIBS = -lc
else
# ubuntu
# tested with https://github.com/yamt/garbage/tree/master/myubuntu
LD = ld.lld -z relro --hash-style=gnu --build-id --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2
LIBS = -L /usr/lib/gcc/x86_64-linux-gnu/9 -L /usr/lib/x86_64-linux-gnu -lgcc -lc
endif

test: $(BIN)
	objdump -r -d $(BIN)
	nm $(BIN)

$(BIN): $(OBJS)
	$(LD) \
	-o $@ \
	/usr/lib/x86_64-linux-gnu/crt1.o \
	/usr/lib/x86_64-linux-gnu/crti.o \
	/usr/lib/gcc/x86_64-linux-gnu/9/crtbegin.o \
	$(OBJS) \
	$(LIBS) \
	/usr/lib/gcc/x86_64-linux-gnu/9/crtend.o \
	/usr/lib/x86_64-linux-gnu/crtn.o

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
