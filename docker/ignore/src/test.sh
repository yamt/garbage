#! /bin/sh

# Note: this needs buildkit backend.
# https://docs.docker.com/engine/reference/commandline/build/#use-a-dockerignore-file

docker build -f Dockerfile -t foo ..
docker build -f Dockerfile.bar -t bar ..

echo "==== foo ==="
docker run foo
echo "==== bar ==="
docker run bar
