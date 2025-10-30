import numpy as np
import itertools
import random
import time

import model

# import torch_model as model

# https://www.askpython.com/python/examples/load-and-plot-mnist-dataset-in-python
from keras.datasets import mnist

(train_data, train_answers), (test_data, test_answers) = mnist.load_data()
test_answers = test_answers.astype(int)
train_data = train_data.reshape(-1, 28 * 28).astype(np.float32) / 255.0
train_answers = train_answers.astype(int)
test_data = test_data.reshape(-1, 28 * 28).astype(np.float32) / 255.0


print(f"train_data shape {train_data.shape}")
print(f"train_answers shape {train_answers.shape}")
print(f"test_data shape {test_data.shape}")
print(f"test_answers shape {test_answers.shape}")


n = model.Network([28 * 28, 30, 10])
r = model.test(n, test_data, test_answers)
print(f"initial accuracy: {r} ({r / len(test_data) * 100:.2f}%) (expected: 10%)")
print(model.feed_forward(n, test_data[:1]))


def chunk(it, n):
    i = iter(it)
    return iter(lambda: list(itertools.islice(i, n)), [])


def onehot(a):
    return np.identity(10, dtype=np.float32)[a]


train_answers_a = onehot(train_answers)
assert train_answers_a.shape == (train_answers.shape[0], 10)

learning_rate = 3.0
batch_size = 10
epoches = 30

ix = list(range(0, train_data.shape[0]))
for e in range(0, epoches):
    print(f"epoch {e} start")
    start_time = time.perf_counter()
    random.shuffle(ix)
    for ch in chunk(ix, batch_size):
        model.sgd(n, train_data[ch], train_answers_a[ch], learning_rate)
    r = model.test(n, test_data, test_answers)
    total = len(test_data)
    end_time = time.perf_counter()
    print(
        f"epoch {e} end, {end_time - start_time:.2f} sec, {r}/{total} ({r / total * 100:.2f}%) (data)"
    )
    # for ch in chunk(data, 10000):
    #     a, b = zip(*ch)
    #     r = model.test(n, a, [np.argmax(c) for c in b])
    #     print(f"epoch {e} end, {r} (train)")
    # for i in range(0, 2):
    #     print(model.feed_forward(n, test_data[i]))
    #     print(test_answers[i])


for ix in range(10):
    r = model.feed_forward(n, test_data[ix : ix + 1])[0]
    # print(f"{r}")
    ans_ix = np.argmax(r)
    print(f"[{ix}] expected {test_answers[ix]} actual {ans_ix} ({r[ans_ix]:.3f})")


def plot_errors():
    from matplotlib import pyplot as plt

    def plot(ax, d, ix, desired, actual):
        c = d.copy().reshape(28, 28)
        ax.imshow(c, cmap=plt.get_cmap("gray"))
        ax.set_title(f"ix {ix}\nlabel {desired}\ninferred {actual}")

    data = list(zip(test_data, test_answers))
    # random.shuffle(data)
    rows = 5
    cols = 10
    fig, axes = plt.subplots(rows, cols, squeeze=False, tight_layout=True)
    x = y = 0
    for ix, (d, desired) in enumerate(data):
        ax = axes[y, x]
        r = model.feed_forward(n, d.reshape(1, -1))[0]
        actual = np.argmax(r)
        if desired == actual:
            continue
        print(f"[{ix}] {r}")
        plot(ax, d, ix, desired, actual)
        x += 1
        if x >= cols:
            x = 0
            y += 1
            if y >= rows:
                break
    plt.show()


# plot_errors()
