SRCS = a.m
BIN = test

OBJS = $(SRCS:.m=.o)

.PHONY: run
run: $(BIN)
	./$(BIN)

$(BIN): $(OBJS)
	$(CC) -framework Foundation -framework Cocoa -o $@ $(OBJS)

.m.o:
	$(CC) -c -Wall -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
