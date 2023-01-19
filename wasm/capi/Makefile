WAMR_DIR = $(HOME)/git/wasm-micro-runtime
WAMR_LIB_DIR = $(WAMR_DIR)/product-mini/platforms/darwin/b

V8_DIR = ${HOME}/git/v8/v8
WASMER_DIR= /tmp/wasmer

CC = clang
#CC = ${V8_DIR}/third_party/llvm-build/Release+Asserts/bin/clang
#CXX = clang++
#CXX = ${V8_DIR}/third_party/llvm-build/Release+Asserts/bin/clang++
CFLAGS = -Os -g
#CFLAGS += -I$(WAMR_DIR)/core/iwasm/include
#CFLAGS += -I$(V8_DIR)/third_party/wasm-api
CFLAGS += -I$(WASMER_DIR)/include
CFLAGS += -Wall -Werror

#CLINKFLAGS = -L$(WAMR_LIB_DIR) -lvmlib

#CLINKFLAGS = -L${V8_DIR}/out/wee8/obj
#CLINKFLAGS += -lwee8 -lv8_libplatform -lv8_libbase
#CLINKFLAGS += -std=c++14 -pthread

CLINKFLAGS = -L$(WASMER_DIR)/lib -lwasmer
CLINKFLAGS += -Xlinker -rpath -Xlinker $(WASMER_DIR)/lib

#CFLAGS += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/
#CLINKFLAGS += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/

OBJS = test.o
OBJS += fileio.o
BIN = test

.PHONY: run
run: $(BIN)
	./$(BIN)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN): $(OBJS)
	$(CXX) \
    -o $@ $(OBJS) \
    $(CLINKFLAGS)

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
