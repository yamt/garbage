import numpy as np
from scipy.special import expit


def sigmoid(x):
    # return 1.0 / (1.0 + np.exp(-x))
    return expit(x)


def sigmoid_prime(x):
    s = sigmoid(x)
    return s * (1.0 - s)


def dot(a, b):
    return np.dot(a, b)


class Network:
    def __init__(self, sizes):
        self.biases = [np.random.randn(y, 1) for y in sizes[1:]]
        self.weights = [np.random.randn(y, x) for x, y in zip(sizes[:-1], sizes[1:])]


def feed_forward_full(n, d):
    assert d.ndim == 2
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
    r = [np.argmax(feed_forward(n, d.reshape(28 * 28, 1))) for d in data]
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
    return output - desired


def back_propagation(n, d, desired):
    acts, zs = feed_forward_full(n, d)

    n_w = [None] * len(n.weights)
    n_b = [None] * len(n.biases)
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


def sgd(n, data, answers, rate):
    assert data.ndim == 2
    assert answers.ndim == 2
    batch_size = data.shape[0]
    assert answers.shape[0] == batch_size
    rate = rate / batch_size
    data = data.T
    answers = answers.T
    delta_n_w, delta_n_b = back_propagation(n, data, answers)
    n_w = delta_n_w
    n_b = [np.sum(a, axis=1).reshape(-1, 1) for a in delta_n_b]
    n.weights = sub_list_rate(n.weights, n_w, rate)
    n.biases = sub_list_rate(n.biases, n_b, rate)
