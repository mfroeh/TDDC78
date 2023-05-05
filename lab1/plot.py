import matplotlib.pyplot as plt

x = [1, 2, 4, 8, 16, 32, 64]
blur_pthreads = [16.3786, 8.55247, 4.32111, 2.21067, 1.26054, 0.982082, 1.17529]
threshold_mpi = [0.088511, 0.062794, 0.031244, 0.020858, 0.016567, 0.013478, 0.008912]
threshold_pthreads = [0.109217, 0.0535817, 0.0371549, 0.0252247, 0.0157444, 0.0183072, 0.0130858]
blur_mpi = [14.018321, 7.058290, 3.750033, 1.937216, 1.107606, 0.754410, 0.656233]
lapl_omp = [0.181218, 0.104422, 0.064208, 0.043191, 0.034682, 0.051359]

# create subplots
fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(12, 4))

# plot for blur
ax1.plot(x, blur_pthreads, 'ro-', label='Pthreads')
ax1.plot(x, blur_mpi, 'go-', label='MPI')
ax1.set_title('Blur Execution Time')
ax1.set_xlabel('Threads/Processes')
ax1.set_ylabel('Execution Time (s)')
ax1.legend()

# plot for threshold
ax2.plot(x, threshold_pthreads, 'ro-', label='Pthreads')
ax2.plot(x, threshold_mpi, 'go-', label='MPI')
ax2.set_title('Threshold Execution Time')
ax2.set_xlabel('Threads/Processes')
ax2.set_ylabel('Execution Time (s)')
ax2.legend()

# plot for lapl_omp
ax3.plot(x, lapl_omp, 'bo-', label='OpenMP')
ax3.set_title('Laplacian Execution Time')
ax3.set_xlabel('Threads')
ax3.set_ylabel('Execution Time (s)')
ax3.legend()

plt.show()
