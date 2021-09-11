CC = cc

UNAME = $(shell uname -s)
CFLAGS = -Os
CFLAGS += -Wall -Werror
LIBS =

ifneq ($(UNAME),Darwin)
CLINKFLAGS = -rdynamic
LIBS += -ldl
endif

OBJS = main.o
BIN = test
MOD = ../so/test.so

.PHONY: run
run: $(BIN) $(MOD)
	./$(BIN) $(MOD)

$(MOD):
	$(MAKE) -C ../so

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN): $(OBJS)
	$(CC) $(CLINKFLAGS) -o $@ $< $(LIBS)

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
	$(MAKE) -C ../so clean
