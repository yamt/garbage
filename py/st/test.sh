#! /bin/sh

set -e

TMPFILE=$(mktemp)
trap "rm ${TMPFILE}" 0
python3 st.py < testdata.txt > ${TMPFILE}
diff -upd expected.txt ${TMPFILE}
echo "Success"
