import matplotlib.pyplot as plt
import numpy as np

# Data
x = [1, 2, 4, 8, 16, 32, 64]
blur = [16.3786, 8.55247, 4.32111, 2.21067, 1.26054, 0.982082, 1.17529]
threshold = [0.088511, 0.062794, 0.031244, 0.020858, 0.016567, 0.013478, 0.008912]

# Create figure and two subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 8))

# Plot 1: Blur filter
ax1.plot(x, blur)
ax1.set_title('Blur Filter')
ax1.set_xlabel('X')
ax1.set_ylabel('Y')

# Plot 2: Threshold filter
ax2.plot(x, threshold)
ax2.set_title('Threshold Filter')
ax2.set_xlabel('X')
ax2.set_ylabel('Y')

# Adjust spacing between subplots
plt.subplots_adjust(hspace=0.5)

# Show plot
plt.show()

