CC = clang
CFLAGS = -Os
CFLAGS += -Wall -Werror

OBJS = main.o
BIN = test
MOD = ../so/test.so

.PHONY: run
run: $(BIN) $(MOD)
	./$(BIN) $(MOD)

$(MOD):
	make -C ../so

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN): $(OBJS)
	$(CC) $(CLINKFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
