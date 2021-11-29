import numpy as np
import itertools
import random
from scipy.special import expit


def sigmoid(x):
    # return 1.0 / (1.0 + np.exp(-x))
    return expit(x)


def sigmoid_prime(x):
    s = sigmoid(x)
    return s * (1.0 - s)


def dot(a, b):
    assert a.shape[1] == b.shape[0]
    return np.dot(a, b)


class Network:
    def __init__(self, sizes):
        self.biases = [np.random.randn(y, 1) for y in sizes[1:]]
        self.weights = [np.random.randn(y, x) for x, y in zip(sizes[:-1], sizes[1:])]


def feed_forward_full(n, d):
    acts = []
    zs = []
    a = d
    acts.append(a)
    for w, b in zip(n.weights, n.biases):
        assert len(w) == len(b)
        assert len(b[0]) == 1
        z = dot(w, a) + b
        zs.append(z)
        a = sigmoid(z)
        acts.append(a)

    assert len(acts) - 1 == len(zs)
    return (acts, zs)


def feed_forward(n, a):
    acts, zs = feed_forward_full(n, a)
    return acts[-1]


# def feed_forward(n, a):
#    for w, b in zip(n.weights, n.biases):
#        assert len(w) == len(b)
#        assert len(b[0]) == 1
#        z = dot(w, a) + b
#        a = sigmoid(z)
#    return a


def test(n, data, answers):
    assert len(data) == len(answers)
    r = [np.argmax(feed_forward(n, d)) for d in data]
    return sum(int(a == b) for a, b in zip(r, answers))


def assert_same_shape(la, lb):
    for a, b in zip(la, lb):
        assert a.shape == b.shape


def add_list(a, b):
    assert_same_shape(a, b)
    return [x + y for x, y in zip(a, b)]


def sub_list_rate(a, b, rate):
    assert_same_shape(a, b)
    return [x - rate * y for x, y in zip(a, b)]


def cost_derivative(output, desired):
    # partial derivative of quadratic cost function
    assert_same_shape(output, desired)
    return output - desired


def back_propagation(n, d, desired):
    n_w = [None] * len(n.weights)
    n_b = [None] * len(n.biases)

    acts, zs = feed_forward_full(n, d)
    assert len(acts) - 1 == len(zs) == len(n_w) == len(n_b)

    # note: "*" here is a hadamard product
    delta = cost_derivative(acts[-1], desired) * sigmoid_prime(zs[-1])
    n_w[-1] = dot(delta, acts[-1 - 1].T)
    n_b[-1] = delta
    for l in range(2, len(acts)):
        delta = dot(n.weights[-l + 1].T, delta) * sigmoid_prime(zs[-l])
        n_w[-l] = dot(delta, acts[-l - 1].T)
        n_b[-l] = delta

    return (n_w, n_b)


def sgd(n, data, rate):
    n_w = [np.zeros(x.shape) for x in n.weights]
    n_b = [np.zeros(x.shape) for x in n.biases]
    for d, a in data:
        delta_n_w, delta_n_b = back_propagation(n, d, a)
        n_w = add_list(n_w, delta_n_w)
        n_b = add_list(n_b, delta_n_b)
    n.weights = sub_list_rate(n.weights, n_w, rate)
    n.biases = sub_list_rate(n.biases, n_b, rate)


# https://www.askpython.com/python/examples/load-and-plot-mnist-dataset-in-python
from keras.datasets import mnist

(train_data, train_answers), (test_data, test_answers) = mnist.load_data()
train_data = [np.array(d).reshape(28 * 28, 1) / 255.0 for d in train_data]
test_data = [np.array(d).reshape(28 * 28, 1) / 255.0 for d in test_data]

from matplotlib import pyplot as plt


def plot(d):
    c = d.copy().reshape(28, 28)
    plt.imshow(c, cmap=plt.get_cmap("gray"))
    plt.show()


n = Network([28 * 28, 30, 10])
# r = test(n, test_data, test_answers)
# print(r)  # expected to be 10% or so
print(feed_forward(n, test_data[0]))


def chunk(it, n):
    i = iter(it)
    return iter(lambda: list(itertools.islice(i, n)), [])


targets = {}
for i in range(0, 10):
    targets[i] = np.zeros((10, 1))
    targets[i][i] = 1.0
train_answers_a = [targets[x] for x in train_answers]

learning_rate = 3.0
batch_size = 10
epoches = 30

data = list(zip(train_data, train_answers_a))
for e in range(0, epoches):
    print(f"epoch {e} start")
    random.shuffle(data)
    for ch in chunk(data, batch_size):
        sgd(n, ch, learning_rate / batch_size)
    r = test(n, test_data, test_answers)
    print(f"epoch {e} end, {r}/{len(test_data)} (data)")
    r = test(n, train_data, train_answers)
    print(f"epoch {e} end, {r}/{len(train_data)} (train)")
    for i in range(0, 2):
        print(feed_forward(n, test_data[i]))
        print(test_answers[i])


from matplotlib import pyplot as plt


def plot(ax, d, desired, actual):
    c = d.copy().reshape(28, 28)
    ax.imshow(c, cmap=plt.get_cmap("gray"))
    ax.set_title(f"correct {desired}\ninferred {actual}")


data = list(zip(test_data, test_answers))
random.shuffle(data)
rows = 5
cols = 5
fig, axes = plt.subplots(rows, cols, squeeze=False, tight_layout=True)
x = y = 0
for d, desired in itertools.islice(data, rows * cols):
    ax = axes[y, x]
    actual = np.argmax(feed_forward(n, d))
    plot(ax, d, desired, actual)
    x += 1
    if x >= cols:
        x = 0
        y += 1
plt.show()
