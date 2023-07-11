import numpy as np
import numpy.linalg as LA
import matplotlib.pyplot as plt

x_ori = np.zeros((5, 5), np.float64)
for i in range(5):
    for j in range(4, -1, -1):
        x_ori[i, 4 - j] = np.float_power(i * 5 + 10, -j)
print(x_ori)
x_inv = LA.inv(x_ori)
for i in range(x_inv.size):
    print(x_inv[i // 5][i % 5], end="f, ")