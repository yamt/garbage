#! /bin/sh

exec docker container run \
--rm -it \
--platform linux/amd64 \
-v $(pwd):/work \
yamt/ubuntu \
"$@"
