WASI_SDK_DIR = /opt/wasi-sdk-16.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += -fno-builtin-printf
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror
CFLAGS += -g

SYSROOT = ${HOME}/git/wasi-libc/sysroot
ifeq ($(THREAD),1)
# make -j4 CC=/opt/wasi-sdk-16.0/bin/clang THREAD_MODEL=posix
CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -pthread
#CFLAGS += -mbulk-memory
#CFLAGS += -matomics
CLINKFLAGS += -pthread
CLINKFLAGS += --sysroot $(SYSROOT)
endif

#CLINKFLAGS += -Wl,--allow-undefined

CLINKFLAGS += -z stack-size=$(STACK_SIZE)
CLINKFLAGS += -Wl,--initial-memory=$(INITIAL_MEMORY)
#CLINKFLAGS += -Wl,--max-memory=$(MAX_MEMORY)
#CLINKFLAGS += -Wl,--export=__data_end -Wl,--export=__heap_base
#CLINKFLAGS += -Wl,--export=malloc -Wl,--export=free

STACK_SIZE = 8192
INITIAL_MEMORY = 65536
#MAX_MEMORY = 65536

WAMRC = wamrc
WAMRC.xtensa = wamrc.xtensa

#WAMRC_FLAGS += --enable-perf-profiling

OBJS = hello.o
BIN = hello.wasm

BINS = $(BIN)
WAMR_AOT_BINS += $(BIN).aot-default
WAMR_AOT_BINS += $(BIN).aot-default.xtensa
WAMR_AOT_BINS += $(BIN).aot-xip
WAMR_AOT_BINS += $(BIN).aot-xip.xtensa
WAMR_AOT_BINS += $(BIN).aot-xip.obj
WAMR_AOT_BINS += $(BIN).aot-xip.obj.xtensa

all: $(BINS)

aot: $(WAMR_AOT_BINS)

$(BIN): $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-v \
	-o $@ $<

$(BIN).aot-default: $(BIN)
	$(WAMRC) \
	$(WAMRC_FLAGS) \
	-o $@ $<

$(BIN).aot-default.xtensa: $(BIN)
	$(WAMRC.xtensa) \
	$(WAMRC_FLAGS) \
	--target=xtensa \
	-o $@ $<

$(BIN).aot-xip: $(BIN)
	$(WAMRC) \
	$(WAMRC_FLAGS) \
	--enable-indirect-mode \
	--disable-llvm-intrinsics \
	-o $@ $<

$(BIN).aot-xip.xtensa: $(BIN)
	$(WAMRC.xtensa) \
	$(WAMRC_FLAGS) \
	--enable-indirect-mode \
	--disable-llvm-intrinsics \
	--target=xtensa \
	-o $@ $<

$(BIN).aot-xip.obj: $(BIN)
	$(WAMRC) \
	$(WAMRC_FLAGS) \
	--enable-indirect-mode \
	--disable-llvm-intrinsics \
	--format=object \
	-o $@ $<

$(BIN).aot-xip.obj.xtensa: $(BIN)
	$(WAMRC.xtensa) \
	$(WAMRC_FLAGS) \
	--enable-indirect-mode \
	--disable-llvm-intrinsics \
	--target=xtensa \
	--format=object \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@
	$(CC) -S $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(WAMR_AOT_BINS)
	rm -f $(BINS)
	rm -f $(OBJS)
