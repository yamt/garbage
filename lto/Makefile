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
LD_SO = /lib64/ld-linux-x86-64.so.2
LIB_DIR = /usr/lib/x86_64-linux-gnu
ifeq ($(USE_GCC),)
CC = clang-11
LD = ld.lld-11 -dynamic-linker ${LD_SO}
else
# http://gcc.gnu.org/wiki/whopr/driver
GCC_LTO_PLUGIN_DIR = /usr/lib/gcc/x86_64-linux-gnu/9/
CC = gcc
LD = COLLECT_GCC=$(CC) COLLECT_GCC_OPTIONS= \
ld.gold -dynamic-linker ${LD_SO} \
-plugin ${GCC_LTO_PLUGIN_DIR}/liblto_plugin.so \
-plugin-opt=${GCC_LTO_PLUGIN_DIR}/lto-wrapper \
-plugin-opt=-fresolution=/tmp/a.res
endif
CRT_START = ${LIB_DIR}/crt1.o
CRT_PROLOGUE = ${LIB_DIR}/crti.o
CRT_EPILOGUE = ${LIB_DIR}/crtn.o
LIBS = -L ${LIB_DIR} -lc
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
