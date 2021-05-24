#! /bin/sh

set -e

TMP=$(mktemp -d /tmp/$(basename $0)-XXXXXX)

PLATFORMS="${PLATFORMS} linux/amd64"
PLATFORMS="${PLATFORMS} linux/arm64"
PLATFORMS="${PLATFORMS} linux/arm/v7"

# qemu doesn't implement iptables socket options.
DOCKERD_OPTIONS=--iptables=false

for P in ${PLATFORMS}; do
	CIDFILE=${TMP}/cid

    echo "Testing ${P}..."
    docker container run --platform ${P} \
    --cidfile ${CIDFILE} \
    -d \
    --privileged \
    docker:dind "${DOCKERD_OPTIONS}"

    CID=$(cat ${CIDFILE})

	while ! docker container exec ${CID} docker version > /dev/null 2>&1; do
        echo "Waiting for dockerd to get ready..."
        sleep 1
	done

	docker container exec ${CID} docker container run alpine \
    sh -c "echo hello - \$(uname -a)"

    docker container stop ${CID}
    docker container rm ${CID}
	rm ${CIDFILE}
done
