q = 314
#q = 10

def f(x):
	r = 1
	for k in range(0, x+1):
		r *= 3 + 2 * k
	return r

def mul(l):
	r = 1
	for x in l:
		r *= x
	return r

n = q 
l = []
for i in range(1, n):
	x = (n - i) // (2 ** i)
	if x == 0:
		break
	l.append(x - 1)

print(l)
#l = [ 155, 77, 37, 18, 8, 3, 1, 0, ]

l = [f(x) for x in l]

print(l)
a1 = mul(l)
print(a1)

def kaijo(n):
	r = 1
	for k in range(1, n + 1):
		r *= k
	return r

n0 = kaijo(q)
print(n0)

a2 = n0
while a2 % 2 == 0:
	a2 //= 2
print(a2)

print("--")
print(f"{n0 // a1:x}")
print(n0 // a1)
print(n0 % a1)

print("--")
print(f"{n0 // a2:x}")
print(n0 // a2)
print(n0 % a2)

print("--")
print(a1 // a2)

