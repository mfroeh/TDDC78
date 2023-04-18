import matplotlib.pyplot as plt
import numpy as np

# Data
x = [1, 2, 4, 8, 16, 32, 64]
blur = [16.3786, 8.55247, 4.32111, 2.21067, 1.26054, 0.982082, 1.17529]
threshold = [0.088511, 0.062794, 0.031244, 0.020858, 0.016567, 0.013478, 0.008912]

# Plot
fig, ax = plt.subplots()
ax.plot(x, blur, label='Blur filter')
ax.plot(x, threshold, label='Threshold filter')

# Add labels and title
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_title('Blur and Threshold Filters')

# Add legend
ax.legend()

# Show plot
plt.show()

