import pandas as pd
import matplotlib.pyplot as plt

# Load the data
df = pd.read_csv('weak.csv')

# Calculate the baselines of matching and total
baseline_matching = df[df['processes'] == 1]['matching'].mean()
df['total'] = df['read'] + df['generation'] + df['matching']
baseline_total = df[df['processes'] == 1]['total'].mean()

# Calculate mean, min, max
average_matching = df.groupby('processes')['matching'].mean()
min_matching = df.groupby('processes')['matching'].min()
max_matching = df.groupby('processes')['matching'].max()
average_total = df.groupby('processes')['total'].mean()
min_total = df.groupby('processes')['total'].min()
max_total = df.groupby('processes')['total'].max()

# Calculate efficiencies
efficiency_matching = average_matching / (baseline_matching * df['processes'].unique())
efficiency_min_matching = min_matching / (baseline_matching * df['processes'].unique())
efficiency_max_matching = max_matching / (baseline_matching * df['processes'].unique())
efficiency_total = average_total / (baseline_total * df['processes'].unique())
efficiency_min_total = min_total / (baseline_total * df['processes'].unique())
efficiency_max_total = max_total / (baseline_total * df['processes']. unique())

# Plotting
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# Efficiency for matching
ax1.plot(df['processes'].unique(), efficiency_matching, marker='o', color='blue')
ax1.fill_between(df['processes'].unique(), efficiency_min_matching, efficiency_max_matching, color='blue', alpha=0.1)
ax1.set_title('Efficiency for Matching')
ax1.set_xlabel('Number of Processes')
ax1.set_ylabel('Efficiency')
ax1.grid(True)

# Efficiency for all phases
ax2.plot(df['processes'].unique(), efficiency_total, marker='o', color='red')
ax2.fill_between(df['processes'].unique(), efficiency_min_total, efficiency_max_total, color='red', alpha=0.1)
ax2.set_title('Efficiency for all phase')
ax2.set_xlabel('Number of Processes')
ax2.set_ylabel('Efficiency')
ax2.grid(True)

plt.tight_layout()
plt.savefig('weak_scaling.png')