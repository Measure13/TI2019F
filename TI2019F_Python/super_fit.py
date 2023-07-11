import numpy as np
import numpy.linalg as LA
import matplotlib.pyplot as plt

MAX_PAPER_NUM = 30
POLY = 5
y = np.zeros(MAX_PAPER_NUM, np.float64)
x_ori = np.zeros((POLY, POLY), np.float64)
for i in range(POLY):
    for j in range(4, -1, -1):
        x_ori[i, 4 - j] = np.float_power(i * POLY + 10, -j)
x_inv = LA.inv(x_ori)
# Y_5 = np.array([110, 85, 70, 60, 56])
# Y_5 = Y_5 * 84 * 2
Y_5 = np.array([0x23d1, 0x1b90, 0x16a5, 0x1354, 0x1206])
coef = x_inv @ Y_5
X_30 = np.ones((MAX_PAPER_NUM + 1, POLY), np.float64)
for i in range(1, MAX_PAPER_NUM + 1):
    for j in range(POLY):
        X_30[i][j] = 1 / np.float_power(i, POLY - 1 - j)
Y_30 = X_30 @ coef
print(coef)
print(X_30)
print(Y_30)
plt.plot(np.arange(1, 31), Y_30[np.arange(1, 31)], label="compute")
plt.scatter(np.arange(10, 31, 5), Y_5, 15, "r", "*", label="input")
plt.legend()
plt.show()
# 5     170     3797
# 10    110     23d1
# 15    85      1b90
# 20    70      16a5
# 25    60      1354
# 30    56      1206