BIN = dksync
OBJS = main.o
OBJS += syncer.o
OBJS += hotplug.o
CFLAGS = -Os

# darwin
OBJS += power.o
LIBS += -framework IOKit
LIBS += -framework CoreFoundation

LIBUSB = libusb-1.0
CFLAGS += $(shell pkg-config --cflags $(LIBUSB))
LIBS += $(shell pkg-config --libs $(LIBUSB))

HIDAPI = hidapi
CFLAGS += $(shell pkg-config --cflags $(HIDAPI))
LIBS += $(shell pkg-config --libs $(HIDAPI))

CC = cc

$(BIN): $(OBJS)
	$(CC) \
	-o $@ \
	$(OBJS) \
	$(LIBS)

.c.o:
	$(CC) -c \
	$(CFLAGS) \
	-o $@ \
	-g \
	$<

clean:
	rm -f $(BIN)
	rm -f $(OBJS)
