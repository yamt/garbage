import math


def mean(l):
    return sum(l) / len(l)


def var(l):
    return mean([x**2 for x in l]) - mean(l) ** 2


def std(l):
    return math.sqrt(var(l))


def covar(a, b):
    return mean([x * y for (x, y) in zip(a, b)]) - mean(a) * mean(b)


def correlation(a, b):
    return covar(a, b) / std(a) / std(b)


p = [50, 60, 70, 80, 90]
q = [40, 70, 90, 60, 100]

print(correlation(p, q))
