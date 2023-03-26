import itertools

l = [0,3,1,0,2,0,2,3]
def decode(e):
    k = itertools.cycle(l)
    z = zip(e, k)
    d = [chr(ord('a') + (x - y - 1)) for (x, y) in z]
    return "".join(d)

print(decode([12,18,15,5,14,25,14,4,2,22]))
print(decode([19,19,19,1,27,1,22,22,16,15,10,20,21,2,17,26,12]))
print(decode([19,11,2,20,22,5,20,8,4,22,13,1,4,19]))
print(decode([3,12,17,10,7,18]))
print(decode([19,23,6,1,15,25,21,19,18,9,14,7,19]))
