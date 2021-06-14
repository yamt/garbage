UNAME = $(shell uname -s)

BIN = foo
OBJS = a.o f.o

ifeq ($(UNAME),Darwin)
# macos
# tested on mojave
CC = cc
LD = ld -arch x86_64 -macos_version_min 10.14
CRT_START =
CRT_PROLOGUE =
CRT_EPILOGUE =
LIBS = -lc
else
# ubuntu
# tested with https://github.com/yamt/garbage/tree/master/myubuntu
ifeq ($(USE_GCC),)
CC = clang-11
LD = ld.lld-11 -dynamic-linker /lib64/ld-linux-x86-64.so.2
else
# http://gcc.gnu.org/wiki/whopr/driver
CC = gcc
LD = COLLECT_GCC=$(CC) COLLECT_GCC_OPTIONS= \
ld.gold -dynamic-linker /lib64/ld-linux-x86-64.so.2 \
-plugin /usr/lib/gcc/x86_64-linux-gnu/9/liblto_plugin.so \
-plugin-opt=/usr/lib/gcc/x86_64-linux-gnu/9/lto-wrapper \
-plugin-opt=-fresolution=/tmp/a.res
endif
CRT_START = /usr/lib/x86_64-linux-gnu/crt1.o
CRT_PROLOGUE = /usr/lib/x86_64-linux-gnu/crti.o
CRT_EPILOGUE = /usr/lib/x86_64-linux-gnu/crtn.o
LIBS = -L /usr/lib/x86_64-linux-gnu -lc
endif

test: $(BIN)
	./$(BIN)
	objdump -r -d $(BIN)
	nm $(BIN)

$(BIN): $(OBJS)
	$(LD) \
	-o $@ \
	$(CRT_START) \
	$(CRT_PROLOGUE) \
	$(OBJS) \
	$(LIBS) \
	$(CRT_EPILOGUE)

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
