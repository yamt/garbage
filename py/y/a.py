import numpy as np

x = np.random.rand(1,2)
w = np.random.rand(2,3)

y = np.dot(x,w)
print(x.shape, x, type(x))
print(w.shape, w, type(w))
print(y.shape, y, type(y))
