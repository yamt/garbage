def f(a):
    n = len(a)
    m = 3
    for i in range(0, n - m):
        sub = a[i : i + m]
        rest = a[:i] + a[i + m:]
        yield sub + rest
        yield rest + sub

def g(l):
    for a in l:
        for n in f(a[-1]):
            yield a + [n]


s = "123456"
l = [[s]]
for x in range(1, 6):
    l = g(l)
l = [a for a in l if a[-1] == s[::-1]]
print(list(l))
