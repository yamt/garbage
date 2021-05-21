#! /bin/sh

set -e

PLATFORMS="${PLATFORMS} linux/amd64"
PLATFORMS="${PLATFORMS} linux/arm64"
PLATFORMS="${PLATFORMS} linux/arm/v7"

for P in ${PLATFORMS}; do
	# good:
	#	exe: applet not found
	# bad:
	#	qemu: no user program specified
    echo "Testing ${P}..."
	docker run --rm --platform ${P} alpine sh -c "/proc/1/exe --version" 2>&1 |
    fgrep "exe: applet not found" > /dev/null \
    && echo "=> good" || (echo "=> bad"; exit 1)
done
