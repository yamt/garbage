
def f(x):
    res = ""
    c = None
    n = 0
    while x:
        h = x[0]
        x = x[1:]
        if n == 0:
            c = h
        if c == h:
            n += 1
        else:
            res += str(n) + c
            c = h
            n = 1
    res += str(n) + c
    return res

assert f("22") == "22"
assert f("222") == "32"
assert f("1") == "11"
assert f("12") == "1112"

a = "2"
while True:
    print(a, len(a))
    a = f(a)
