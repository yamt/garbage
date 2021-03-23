#! /bin/sh

set -e

GIT=${GIT:-git}

DIR=$(mktemp -d)
echo "Using $DIR"
cd ${DIR}

${GIT} init

echo foo > file1
${GIT} add file1
${GIT} commit -m "add file1"

echo cow is here > file1
${GIT} diff > a.diff
${GIT} reset --hard

echo bar > file2
${GIT} add file2
${GIT} commit -F - <<EOF
hey

this is a commit message containing a patch
$(cat a.diff)
EOF

${GIT} format-patch -o p HEAD^!

echo "------------------------ Generated patch"
cat p/0001*
echo "------------------------ Generated patch (end)"

${GIT} reset --hard HEAD~
${GIT} am p/0001-*

echo "------------------------ Applied patch"
${GIT} show
echo "------------------------ Applied patch (end)"

echo "------------------------ file1"
cat file1
