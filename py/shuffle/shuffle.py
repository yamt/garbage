#! /usr/bin/env python3

# something like shuffle(1)

import random
import sys

l = sys.argv[1:]
random.shuffle(l)
for x in l:
    print(x)
