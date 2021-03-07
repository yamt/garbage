
# pip install --user matplotlib
# pip install --user parse

# input lines look like:
#  2021-03-04 00:05:05.355812+00:00|498331

import sys

import parse

x = []
y = []
p = parse.compile("{ts:ti}|{value:d}\n")
f = sys.stdin
for l in f.readlines():
    r = p.parse(l)
    x.append(r['ts'])
    y.append(r['value'])

import matplotlib.pyplot as plot
from matplotlib.dates import DateFormatter

fig, ax = plot.subplots()
ax.xaxis.set_major_formatter(DateFormatter('%Y-%m-%d %H:%M'))
ax.plot(x, y, 'o-')
labels = ax.get_xticklabels()
plot.setp(labels, rotation=90)
plot.tight_layout()
plot.show()
