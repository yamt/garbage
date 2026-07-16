#! /bin/sh

set -e
set -x

T=${HOME}/git
mkdir -p ${T}

cd ${T}
if [ ! -d garbage ]; then
    git clone https://github.com/yamt/garbage
fi
cd garbage
cd py/git-credential-oauth2
./install.sh
cp examples/dot-gitconfig ${HOME}/.gitconfig

# tweak PATH to use the git credential helpers installed above
# to check out a private repo below.
PATH=${HOME}/.local/bin:${PATH}

cd ${T}
if [ ! -d private-garbage ]; then
    git clone https://gitlab.com/yamt/private-garbage
fi
cd private-garbage
./base/install.sh
find dot-files -type f -name ".*" | while read x; do
    cp $x ${HOME}
done
