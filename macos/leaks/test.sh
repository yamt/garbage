#! /bin/sh

set -e
cc a.c
leaks --outputGraph=x --atExit -- ./a.out 1000
heap -showSizes x.memgraph
