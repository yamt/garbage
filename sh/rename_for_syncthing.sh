#! /bin/sh

set -e

# syncthing doesn't like :?|*
for x in *[:\?\|\*]*; do
    N=$(printf "%s" "$x" | tr ":?|*" _)
    echo "OLD NAME: $x"
    echo "NEW NAME: $N"
    mv -i "$x" "$N"
done
