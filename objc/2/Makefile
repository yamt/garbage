SRCS = a.c
BIN = test

OBJS = $(SRCS:.c=.o)

.PHONY: run
run: $(BIN)
	./$(BIN)

$(BIN): $(OBJS)
	$(CC) -framework Foundation -o $@ $<

.m.o:
	$(CC) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
