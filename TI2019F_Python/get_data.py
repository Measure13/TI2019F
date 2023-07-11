data = [0x3219, 0x206A, 0x18f3, 0x1539, 0x1289, 0x10a4, 0x0f4c, 0x0e4c, 0x0d7d, 0x0cdb, 0x0c53, 0x0be6, 0x0b85, 0x0b36, 0x0aec]
import numpy as np
import numpy.linalg as LA
import matplotlib.pyplot as plt

num_x = 30
data = np.array(data)
y = data[2::3]
x_inv = np.zeros((5, 5), np.float64)
for i in range(3, 16, 3):
    for j in range(4, -1, -1):
        x_inv[i // 3 - 1, 4 - j] = np.float_power(i, -j)
x_inv = LA.inv(x_inv)
print(x_inv)
coef = x_inv @ y
xs = np.zeros((num_x, 5), np.float64)
print(coef)
for i in range(1, num_x + 1):
    for j in range(4, -1, -1):
        xs[i - 1, 4 - j] = np.float_power(i, -j)
y_birb = xs @ coef
print(data[2:] - y_birb[2:15])
# print(np.average(np.float_power(data[2:] - y_birb[2:15], 2)))
# print(np.abs(y_birb[28] - 0x08ea))
# print(np.abs(y_birb[29] - 0x08d5))
plt.plot(data, label="real")
plt.plot(y_birb, label="birb")
plt.scatter(29 - 1, 0x08ea)
plt.scatter(30 - 1, 0x08d5)
plt.legend()
plt.show()