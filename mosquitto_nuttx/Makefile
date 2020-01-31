-include $(TOPDIR)/Make.defs

PROGNAME  = mosquitto
PRIORITY  = SCHED_PRIORITY_DEFAULT
STACKSIZE = 8192
#MODULE    = 

MAINSRC = mosquitto.c

# XXX set ROOTDEPPATH
VPATH = mosquitto$(DELIM)src:mosquitto$(DELIM)lib

CFLAGS += -Imosquitto
CFLAGS += -Imosquitto$(DELIM)lib
CFLAGS += -Imosquitto$(DELIM)src
CFLAGS += -Imosquitto$(DELIM)src$(DELIM)deps
CFLAGS += -DVERSION=\"dummy\"
CFLAGS += -DWITH_BROKER

# hacks to make it build
CFLAGS += -DSIGINT=0
CFLAGS += -DSIGTERM=0
CFLAGS += -DSIGHUP=0
CFLAGS += -Dopenlog\(a,b,c\)
CFLAGS += -Dcloselog\(\)
CFLAGS += -Dfork\(\)
CFLAGS += -Dsetsid\(\)

# hack for sim build
CFLAGS += -U__APPLE__

CSRCS += \
alias_mosq.c \
handle_ping.c \
handle_pubackcomp.c \
handle_pubrec.c \
handle_pubrel.c \
handle_suback.c \
handle_unsuback.c \
memory_mosq.c \
net_mosq.c \
net_mosq_ocsp.c \
packet_datatypes.c \
packet_mosq.c \
property_mosq.c \
send_connect.c \
send_disconnect.c \
send_mosq.c \
send_publish.c \
send_subscribe.c \
send_unsubscribe.c \
time_mosq.c \
tls_mosq.c \
utf8_mosq.c \
util_mosq.c \
util_topic.c \
will_mosq.c \
bridge.c \
conf.c \
conf_includedir.c \
context.c \
database.c \
handle_auth.c \
handle_connack.c \
handle_connect.c \
handle_disconnect.c \
handle_publish.c \
handle_subscribe.c \
handle_unsubscribe.c \
logging.c \
loop.c \
net.c \
persist_read.c \
persist_read_v234.c \
persist_read_v5.c \
persist_write.c \
persist_write_v5.c \
plugin.c \
property_broker.c \
read_handle.c \
security.c \
security_default.c \
send_auth.c \
send_connack.c \
send_suback.c \
send_unsuback.c \
session_expiry.c \
signals.c \
subs.c \
sys_tree.c \
websockets.c \
will_delay.c

include $(APPDIR)/Application.mk
