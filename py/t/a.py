def f(a):
    yield a[0:1] + a[4 : 4 + 2] + a[1 : 1 + 3]
    yield a[1 : 1 + 3] + a[0:1] + a[4 : 4 + 2]
    yield a[0:2] + a[5 : 5 + 1] + a[2 : 2 + 3]
    yield a[2 : 2 + 3] + a[0:2] + a[5 : 5 + 1]
    yield a[3 : 3 + 6] + a[0:3]


def g(l):
    for a in l:
        for n in f(a[-1]):
            yield a + [n]


l = [["123456"]]
for x in range(1, 6):
    l = g(l)
# l = [a for a in l if a[2] == "126345"]
l = [a for a in l if a[-1] == "654321"]
print(list(l))
