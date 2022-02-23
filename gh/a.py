# pip install ghapi
# pip install python-dateutil
# pip install matplotlib
#
# input environment variables:
#   OWNER
#   REPO
#   OUTPUT
#   GITHUB_TOKEN  (for a private repo)

import os
import itertools

from ghapi.all import GhApi
from ghapi.all import paged
from ghapi.all import print_summary
import dateutil.parser
import matplotlib.pyplot as plot
from matplotlib.ticker import MultipleLocator
import numpy as np

owner = os.environ.get("OWNER", "yamt")
repo = os.environ.get("REPO", "garbage")

api = GhApi(owner=owner, repo=repo)
api.debug = print_summary

# default: sort=created, direction=desc
# XXX should filter on target branch
# XXX esp-idf often uses an awkward way to merge PRs.
#     how can i deal with it?
#     eg. https://github.com/espressif/esp-idf/pull/8248
# XXX for some reasons, state=closed often causes 502
#     for kubernetes/kubernetes.
pgs = paged(api.pulls.list, state="all", per_page=100)
l = itertools.chain.from_iterable(pgs)
l = filter(lambda p: p.merged_at is not None, l)
l = itertools.islice(l, 500)

day_in_sec = 24 * 60 * 60.0
x = dict()
for p in l:
    created_at = dateutil.parser.isoparse(p.created_at)
    merged_at = dateutil.parser.isoparse(p.merged_at)
    d = merged_at - created_at
    author = p.user.login
    print(f"{p.number} {author} {d}")
    if author not in x:
        x[author] = []
    x[author].append(d.total_seconds() / day_in_sec)

x = dict(sorted(x.items(), key=lambda x: len(x[1]), reverse=True))
bin_width = 1.0
max_val = max([np.max(a) for a in x.values()])
# bins = np.arange(0, max_val + bin_width, bin_width)
bins = np.linspace(0, max_val, 100)
fig, ax = plot.subplots()
ax.hist(x.values(), label=list(x.keys()), stacked=True, bins=bins)
# ax.xaxis.set_major_locator(MultipleLocator(7.0))
ax.set_title(f"{owner}/{repo} PR open duration histgram")
ax.set_xlabel("Open Duration (day)")
ax.set_ylabel("# of merged PRs")
ax.set_ylim(bottom=-1)
labels = list(itertools.islice(x.keys(), 16))
plot.legend(labels)
# plot.legend(labels, loc='upper left', bbox_to_anchor=(1, 1))
plot.tight_layout()

output = os.environ.get("OUTPUT")
if output is None:
    plot.show()
else:
    plot.savefig(output)
