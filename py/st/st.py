#! /usr/bin/env python3

# Inspired by https://github.com/nferraz/st

from statistics import stdev, median, mean
from itertools import repeat
import sys

stats = [
    ("N", len),
    ("min", min),
    ("max", max),
    ("mean", mean),
    ("median", median),
    ("stddev", stdev),
]


def default_width(n):
    return 3 if n == "N" else 10


def nfrac(n):
    return "0" if n == "N" else 2


def update_header():
    global header
    ss = [f"{x:{w}s}" for (x, y), w in zip(stats, ws)]
    header = "|".join(ss).strip()


ws = list(default_width(x) for x, y in stats)
update_header()


def format_columns(samples):
    return [f"{y(samples):{w}.{nfrac(x)}f}" for (x, y), w in zip(stats, ws)]


def update_column_widths(samples):
    global ws
    ss = format_columns(samples)
    ws = list(map(len, ss))
    update_header()


def print_header():
    print(header)


def print_stats(samples):
    ss = format_columns(samples)
    s = "|".join(ss)
    print(s)


samples = list(map(float, sys.stdin.readlines()))
update_column_widths(samples)
print_header()
print_stats(samples)
