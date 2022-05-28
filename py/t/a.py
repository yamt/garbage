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
srev = s[::-1]
l = [[s]]
n = 0
while True:
    l = g(l)
    l = list(l)
    n += 1
    l2 = [a for a in l if a[-1] == srev]
    print(n)
    if l2:
        print(list(l2))
        break
