#! /bin/sh

# usage:
# ./fetch-sets.sh https://cdn.netbsd.org/pub/NetBSD/NetBSD-10.1/amd64/binary/sets

URL=$1

dl() {
    echo "downloading $1..."
    curl --silent -LO "${URL}/$1"
}

dl SHA512
cat SHA512 | sed -e 's/.*(\(.*\)) = \(.*\)/\1 \2/' | while read x expected; do
    test -f $x || dl $x
    echo "checking $x..."
    actual=$(openssl sha512 -r $x | cut -d ' ' -f1)
    if [ "${actual}" != "${expected}" ]; then
    	echo "ERROR: digest mismatch"
        exit 1
    fi
done
echo "done!"
