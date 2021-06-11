OBJEXT = .o
HEADOBJ = $(HEADSRC:.c=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))
CXXOBJS = $(CXXSRCS:.cxx=$(OBJEXT))
OBJS = $(COBJS) $(CXXOBJS)

UNAME = $(shell uname -s)

BIN = defer-ctors
ifeq ($(UNAME),Darwin)
HEADSRC = up_macho_init.c
else
HEADSRC = up_sinit_init.c
endif
CXXSRCS = a.cxx

all: test

.SUFFIXES: .cxx

.cxx.o:
	c++ -g -c -o $@ $<
.c.o:
	cc -g -c -o $@ $<

$(BIN): $(HEADOBJ) $(OBJS)
	c++ -o $@ $(HEADOBJ) $(OBJS)

test: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
	rm -f $(HEADOBJ)
	rm -f $(OBJS)
