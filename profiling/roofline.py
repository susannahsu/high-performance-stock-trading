import numpy as np
import matplotlib.pyplot as plt

pi = 1536  # Peak performance
beta = 128  # Memory bandwidth
x_break = pi / beta # Ridge point

flops = 10 * 500 * 10000 + 3 * 271177
# Measurements for serial code
time_s = 9537828751 / 1e6
memory_s = 437892352 + 19755941955840

# Measurements for threading code
time_t = 6692593576 / 1e6
memory_t = 443775424 + 4597800033408

# Measurements for threading+processes code
time_tp = 98026753 / 1e6
memory_tp = 1247726528 + 280036024064

x = np.logspace(-6, 2, 1000)
y_mem_bound = np.minimum(beta * x, pi)
y_compute_bound = np.full_like(x, pi)

# Plot
plt.figure(figsize=(10, 6))
plt.loglog(x, y_mem_bound, label='Attainable Performance', color='blue')
plt.fill_between(x, y_mem_bound, color='skyblue', alpha=0.2)
plt.axvline(x=x_break, color='gray', linestyle=':', label=f'Ridge Point (Ib = {x_break:.2f} Flops/Byte)')
plt.plot(flops/memory_s, flops/10**9/time_s, 'o', label='Serial')
plt.plot(flops/memory_t, flops/10**9/time_t, 'o', label='4 Threads')
plt.plot(flops/memory_tp, flops/10**9/time_tp, 'o', label='4 Threads + 8 Processes')

plt.title('Roofline Model')
plt.xlabel('Operational Intensity [Flop/Byte]')
plt.ylabel('Performance [Gflop/s]')
plt.legend()

plt.tight_layout()
plt.savefig('roofline.png')