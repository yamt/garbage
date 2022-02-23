# pip install ghapi
# pip install python-dateutil
#
# input environment variables:
#   OWNER
#   REPO
#   GITHUB_TOKEN  (for a private repo)

import os
import itertools

from ghapi.all import GhApi
from ghapi.all import paged
import dateutil.parser

owner = os.environ.get("OWNER", "yamt")
repo = os.environ.get("REPO", "garbage")

api = GhApi(owner=owner, repo=repo)

l = itertools.chain.from_iterable(paged(api.pulls.list, state="closed", per_page=100))
l = itertools.islice(l, 500)
for p in l:
    if p.merged_at is None:
        continue
    created_at = dateutil.parser.isoparse(p.created_at)
    merged_at = dateutil.parser.isoparse(p.merged_at)
    d = merged_at - created_at
    author = p.user.login
    print(f"{p.number} {author} {d}")
