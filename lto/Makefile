UNAME = $(shell uname -s)

BIN = foo
OBJS = a.o f.o

CC = cc
ifeq ($(UNAME),Darwin)
LD = ld -arch x86_64 -macos_version_min 10.14
CRT_START =
CRT_PROLOGUE =
CRT_EPILOGUE =
LIBS = -lc
else
# ubuntu
# tested with https://github.com/yamt/garbage/tree/master/myubuntu
LD = ld.lld -dynamic-linker /lib64/ld-linux-x86-64.so.2
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
