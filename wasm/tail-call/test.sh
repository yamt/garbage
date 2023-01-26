CC=${CC:-/opt/wasi-sdk-19.0/bin/clang}
CFLAGS="${CFLAGS} -Wall -g -O3 -mtail-call"

${CC} ${CFLAGS} -c a.c
${CC} ${CFLAGS} -c b.c
${CC} ${CFLAGS} -c main.c

${CC} ${CFLAGS} main.o a.o b.o

