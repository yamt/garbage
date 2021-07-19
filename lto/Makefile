UNAME = $(shell uname -s)

BIN = foo
OBJS = a.o f.o
CFLAGS = -Os

LIBFOO = libfoo.a
LIBFOO_OBJS = lib1.o

ifeq ($(UNAME),Darwin)
# macos
# tested on mojave
CC = cc
LD = ld -arch x86_64 -macos_version_min 10.14
LTO_CFLAGS = -flto
# LTO_CFLAGS = -flto=thin
CRT_START =
CRT_PROLOGUE =
CRT_EPILOGUE =
LIBS = -lc
else
# ubuntu
# tested with https://github.com/yamt/garbage/tree/master/myubuntu
LD_SO = /lib64/ld-linux-x86-64.so.2
LIB_DIR = /usr/lib/x86_64-linux-gnu
CRT_START = ${LIB_DIR}/crt1.o
CRT_PROLOGUE = ${LIB_DIR}/crti.o
CRT_EPILOGUE = ${LIB_DIR}/crtn.o
LIBS = -L ${LIB_DIR} -lc
ifeq ($(USE_GCC),)
CC = clang-11
LD = ld.lld-11 -dynamic-linker ${LD_SO}
LTO_CFLAGS = -flto
# LTO_CFLAGS = -flto=thin
else
# http://gcc.gnu.org/wiki/whopr/driver
GCC_LTO_PLUGIN_DIR = /usr/lib/gcc/x86_64-linux-gnu/9/
# Revisit: use mktemp
RESOLUTION_FILE = /tmp/a.res
CC = gcc
LD = COLLECT_GCC=$(CC) COLLECT_GCC_OPTIONS=$(CFLAGS) \
ld.gold -dynamic-linker ${LD_SO} \
-plugin ${GCC_LTO_PLUGIN_DIR}/liblto_plugin.so \
-plugin-opt=${GCC_LTO_PLUGIN_DIR}/lto-wrapper \
-plugin-opt=-fresolution=${RESOLUTION_FILE}
LTO_CFLAGS = -flto
endif
endif

test: $(BIN)
	./$(BIN)
	objdump -r -d $(BIN)

$(BIN): $(OBJS) $(LIBFOO)
	$(LD) \
	-o $@ \
	$(CRT_START) \
	$(CRT_PROLOGUE) \
	$(OBJS) \
	$(LIBS) \
	$(LIBFOO) \
	$(CRT_EPILOGUE)

.c.o:
	$(CC) -c \
	$(CFLAGS) \
	$(LTO_CFLAGS) \
	-o $@ \
	-g \
	$<

$(LIBFOO): $(LIBFOO_OBJS)
	ar rcs $@ $(LIBFOO_OBJS)

clean:
	rm -f $(BIN)
	rm -f $(OBJS)
	rm -f $(LIBFOO)
	rm -f $(LIBFOO_OBJS)
