OBJEXT = .o
HEADOBJ = $(HEADSRC:.c=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cxx=$(OBJEXT))
OBJS = $(COBJS) $(CXXOBJS)

BIN = defer-ctors
HEADSRC = up_macho_init.c
CXXSRCS = a.cxx

all: test

.SUFFIXES: .cxx

.cxx.o:
	c++ -c -o $@ $<

$(BIN): $(HEADOBJ) $(OBJS)
	c++ -o $@ $(HEADOBJ) $(OBJS)

test: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
	rm -f $(HEADOBJ)
	rm -f $(OBJS)
