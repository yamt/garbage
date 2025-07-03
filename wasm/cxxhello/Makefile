WASI_SDK_DIR = /opt/wasi-sdk-25.9g53551e594386-x86_64-macos
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/
RESOURCE_DIR = $(shell $(WASI_SDK_DIR)/bin/clang -print-resource-dir)

#CC = $(WASI_SDK_DIR)/bin/clang
#CXX = $(WASI_SDK_DIR)/bin/clang++
CC = /Volumes/PortableSSD/llvm/build/bin/clang
CXX = /Volumes/PortableSSD/llvm/build/bin/clang++
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += -fno-builtin-printf
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -fdata-sections
CFLAGS += -ffunction-sections
CFLAGS += -Wall -Werror
CFLAGS += -g
CFLAGS += -mcpu=lime1
CLINKFLAGS += -g
CLINKFLAGS += -Wl,-mllvm,-mcpu=lime1

#SYSROOT = ${HOME}/git/wasi-libc/sysroot
CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -resource-dir=$(RESOURCE_DIR)
CLINKFLAGS += --sysroot $(SYSROOT)
CLINKFLAGS += -resource-dir=$(RESOURCE_DIR)

# Note: wasi-sdk >=24 ships LTO build of libc/libcxx
CFLAGS += -flto=full
CLINKFLAGS += -flto=full
#CLINKFLAGS += -Wl,-mllvm,-enable-merge-functions

CXXFLAGS = $(CFLAGS)
CXXFLAGS += -fno-exceptions
CXXFLAGS += -fno-rtti

CLINKFLAGS += -z stack-size=$(STACK_SIZE)
#CLINKFLAGS += -Wl,--initial-memory=$(INITIAL_MEMORY)
#CLINKFLAGS += -Wl,--max-memory=$(MAX_MEMORY)
#CLINKFLAGS += -Wl,--strip-debug
#CLINKFLAGS += -Wl,--strip-all
CLINKFLAGS += -Wl,--gc-sections
#CLINKFLAGS += -Wl,--lto-CGO2
#CLINKFLAGS += -Wl,--lto-O2
# See llvm::getInlineParams
# -O3 -> 250 (OptAggressiveThreshold)
# -Os -> 50  (OptSizeThreshold)
# -Oz -> 5   (OptMinSizeThreshold)
CLINKFLAGS += -Wl,-mllvm,-inline-threshold=5

STACK_SIZE = 8192
#INITIAL_MEMORY = 6553600
#MAX_MEMORY = 6553600

OBJS = cxxhello.o
BIN = cxxhello.wasm

BINS = $(BIN)

.SUFFIXES: .cxx

all: $(BINS)

$(BIN): $(OBJS)
	$(CXX) \
	$(CLINKFLAGS) \
	-v \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
