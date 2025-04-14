#! /bin/sh

# - ubuntu 20.10 seems to have an official podman package and
#   probably does not require the most of the following stuff
# - tested in a --privileged ubuntu 20.04 docker container on macOS
# - "podman container stats" didn't work for some reasons

apt-get update
apt-get install -y gnupg1 ca-certificates crudini

# https://podman.io/getting-started/installation.html
. /etc/os-release
echo "deb https://download.opensuse.org/repositories/devel:/kubic:/libcontainers:/stable/xUbuntu_${VERSION_ID}/ /" | tee /etc/apt/sources.list.d/devel:kubic:libcontainers:stable.list
curl -L "https://download.opensuse.org/repositories/devel:/kubic:/libcontainers:/stable/xUbuntu_${VERSION_ID}/Release.key" | apt-key add -
apt-get update
apt-get -y upgrade
apt-get -y install podman

touch /etc/containers/containers.conf
crudini --merge /etc/containers/containers.conf < conf/containers.conf
touch /etc/containers/storage.conf
crudini --merge /etc/containers/storage.conf < conf/storage.conf
