# https://github.com/apache/nuttx/pull/10925#discussion_r1365137085

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()
ax1 = fig.add_subplot(1, 1, 1)

data1 = (np.random.randint(0,0x100000000,100000) % 65536) % 32000
data2 = np.random.randint(0,0x100000000,100000) % 32000

ax1.hist(data1, 50, label="(rand % 65536) % 32000")
ax1.hist(data2, 50, label="rand % 32000", alpha=0.5)
ax1.legend()

plt.tight_layout()
plt.savefig("hist.png")
