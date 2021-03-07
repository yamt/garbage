#! /bin/sh

sqlite3 \
-readonly \
yt.db \
"select ts,json_extract(data, '\$.viewCount') from stats order by ts;"
