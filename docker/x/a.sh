#! /bin/sh

docker system prune -a -f
docker image ls
docker buildx build -f Dockerfile.foo -t foo --load .
docker buildx build -f Dockerfile.bar -t bar --load .
docker image ls
