import pandas as pd
import matplotlib.pyplot as plt

# Load the data
df = pd.read_csv('thread.csv')

# Calculate the baselines of matching and total
baseline_matching = df[df['thread'] == 1]['matching'].mean()
df['total'] = df['read'] + df['generation'] + df['matching']
baseline_total = df[df['thread'] == 1]['total'].mean()

# Calculate mean, min, max
average_matching = df.groupby('thread')['matching'].mean()
min_matching = df.groupby('thread')['matching'].min()
max_matching = df.groupby('thread')['matching'].max()
average_total = df.groupby('thread')['total'].mean()
min_total = df.groupby('thread')['total'].min()
max_total = df.groupby('thread')['total'].max()

# Calculate speedups
speedup_matching = baseline_matching / average_matching
speedup_min_matching = baseline_matching / max_matching
speedup_max_matching = baseline_matching / min_matching
speedup_total = baseline_total / average_total
speedup_min_total = baseline_total / max_total
speedup_max_total = baseline_total / min_total

# Plotting
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# Speedup for matching
ax1.plot(average_matching.index, speedup_matching, marker='o', color='blue')
ax1.fill_between(average_matching.index, speedup_min_matching, speedup_max_matching, color='blue', alpha=0.1)
ax1.set_title('Speedup for Matching')
ax1.set_xlabel('Number of Threads')
ax1.set_ylabel('Speedup')
ax1.grid(True)

# Speedup for all phases
ax2.plot(average_total.index, speedup_total, marker='o', color='red')
ax2.fill_between(average_total.index, speedup_min_total, speedup_max_total, color='red', alpha=0.1)
ax2.set_title('Speedup for All Phases')
ax2.set_xlabel('Number of Threads')
ax2.set_ylabel('Speedup')
ax2.grid(True)

plt.tight_layout()
plt.savefig('thread_scaling.png')