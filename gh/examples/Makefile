REPOS += LiamBindle/MQTT-C
REPOS += ARMmbed/mbedtls
REPOS += moby/moby
REPOS += moby/vpnkit
REPOS += thingsboard/thingsboard
REPOS += bytecodealliance/wasm-micro-runtime
REPOS += dvidelabs/flatcc
REPOS += kubernetes/kubernetes

all:

FILES =
.for r in ${REPOS}
F := ${r:S/\//-/}.png
OWNER.${F} := ${r:C/\/.*//}
REPO.${F} := ${r:C/.*\///}
FILES := ${FILES} ${F}

${F}:
	OWNER=${OWNER.${.TARGET}} REPO=${REPO.${.TARGET}} OUTPUT=${.TARGET} python3 ../a.py
.endfor

all: ${FILES}

clean:
	rm -rf ${FILES}
