import numpy as np

# from scipy.special import expit


def sigmoid(x):
    # return expit(x)
    return 1.0 / (1.0 + np.exp(-x))


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
    # acts[0]: inputs of the network
    # acts[l+1]: activations of layer l (l=0,1,2,...)
    # zs[l]: weighted inputs of layer l
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
    acts, zs = feed_forward_full(n, a.T)
    return acts[-1].T


def test(n, data, answers):
    assert len(data) == len(answers)
    r = feed_forward(n, data)
    r = np.argmax(r, axis=1)
    # return sum(int(a == b) for a, b in zip(r, answers))
    return np.sum(r == answers)


def assert_same_shape(la, lb):
    for a, b in zip(la, lb):
        assert a.shape == b.shape


def sub_list_rate(a, b, rate):
    assert_same_shape(a, b)
    return [x - rate * y for x, y in zip(a, b)]


def cost_derivative(output, desired):
    # partial derivative of our cost function.
    #
    # our cost function is quadratic cost function (aka MSE):
    #    C = norm(desired - output) ^ 2 / 2
    return output - desired


def back_propagation(n, d, desired):
    d = d.T
    desired = desired.T
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
    delta_n_w, delta_n_b = back_propagation(n, data, answers)
    n_w = delta_n_w
    n_b = [np.sum(a, axis=1).reshape(-1, 1) for a in delta_n_b]
    n.weights = sub_list_rate(n.weights, n_w, rate)
    n.biases = sub_list_rate(n.biases, n_b, rate)
