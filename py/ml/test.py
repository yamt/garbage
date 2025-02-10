import numpy as np
import itertools
import random
import model

# https://www.askpython.com/python/examples/load-and-plot-mnist-dataset-in-python
from keras.datasets import mnist

(train_data, train_answers), (test_data, test_answers) = mnist.load_data()
train_data = train_data.reshape(-1, 28 * 28).T / 255.0
test_data = [np.array(d).reshape(28 * 28, 1) / 255.0 for d in test_data]


n = model.Network([28 * 28, 30, 10])
# r = model.test(n, test_data, test_answers)
# print(r)  # expected to be 10% or so
# print(model.feed_forward(n, test_data[0]))


def chunk(it, n):
    i = iter(it)
    return iter(lambda: list(itertools.islice(i, n)), [])


def onehot(a):
    return np.identity(10)[a].T


train_answers_a = onehot(train_answers)

learning_rate = 3.0
batch_size = 10
epoches = 30

ix = list(range(0, train_data.shape[1]))
for e in range(0, epoches):
    print(f"epoch {e} start")
    random.shuffle(ix)
    for ch in chunk(ix, batch_size):
        model.sgd(
            n, train_data.T[ch].T, train_answers_a.T[ch].T, learning_rate / batch_size
        )
    r = model.test(n, test_data, test_answers)
    print(f"epoch {e} end, {r}/{len(test_data)} (data)")
    # for ch in chunk(data, 10000):
    #     a, b = zip(*ch)
    #     r = model.test(n, a, [np.argmax(c) for c in b])
    #     print(f"epoch {e} end, {r} (train)")
    for i in range(0, 2):
        print(model.feed_forward(n, test_data[i]))
        print(test_answers[i])


from matplotlib import pyplot as plt


def plot(ax, d, desired, actual):
    c = d.copy().reshape(28, 28)
    ax.imshow(c, cmap=plt.get_cmap("gray"))
    ax.set_title(f"label {desired}\ninferred {actual}")


data = list(zip(test_data, test_answers))
random.shuffle(data)
rows = 5
cols = 10
fig, axes = plt.subplots(rows, cols, squeeze=False, tight_layout=True)
x = y = 0
for d, desired in data:
    ax = axes[y, x]
    actual = np.argmax(model.feed_forward(n, d))
    if desired == actual:
        continue
    plot(ax, d, desired, actual)
    x += 1
    if x >= cols:
        x = 0
        y += 1
        if y >= rows:
            break
plt.show()
