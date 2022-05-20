WASI_SDK_DIR = /tmp/wasi-sdk-14.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += -fno-builtin-printf
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror

WAMRC = wamrc
WAMRC.xtensa = wamrc.xtensa

WAMRC_FLAGS += --enable-perf-profiling

OBJS = hello.o
BIN = test.wasm

BINS = $(BIN)
BINS += $(BIN).aot-default
BINS += $(BIN).aot-default.xtensa
BINS += $(BIN).aot-xip
BINS += $(BIN).aot-xip.xtensa
BINS += $(BIN).aot-xip.obj
BINS += $(BIN).aot-xip.obj.xtensa

all: $(BINS)

$(BIN): $(OBJS)
	$(CC) \
	-Wl,--allow-undefined \
	-z stack-size=8192 \
	-Wl,--initial-memory=65536 \
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
	rm -f $(BINS)
	rm -f $(OBJS)
