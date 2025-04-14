#! /bin/sh

DOCKER_OPTIONS=

while [ $# -ge 1 ]; do
    if [ $1 = -- ]; then
        shift
        break
    fi
    DOCKER_OPTIONS="${DOCKER_OPTIONS} $1"
    shift
done

exec docker container run \
--rm -it \
--platform linux/amd64 \
-v $(pwd):/work \
${DOCKER_OPTIONS} \
yamt/ubuntu-jammy \
"$@"
