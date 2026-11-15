#! /bin/sh

set -e

# syncthing doesn't like :?|*
for x in *[:\?\|\*]*; do
    N=$(printf "%s" "$x" | tr ":?|*" _)
    mv -i "$x" "$N"
done
