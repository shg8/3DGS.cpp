import pandas as pd
import matplotlib.pyplot as plt

# Ask for the path to the CSV file
path = input('Enter the path to the CSV file: ')
df = pd.read_csv(path)

# Convert nanoseconds to milliseconds
for col in ['preprocess', 'prefix_sum', 'preprocess_sort', 'sort', 'tile_boundary', 'render']:
    df[col] = df[col] / 1e6  # Convert from ns to ms

# Sort DataFrame by num_instances
df = df.sort_values(by='num_instances')

# Order the kernels according to the execution sequence
kernel_sequence = ['preprocess', 'prefix_sum', 'preprocess_sort', 'sort', 'tile_boundary', 'render']

# Calculate cumulative sums for the specified order
df_cumulative = df.copy()
for i, kernel in enumerate(kernel_sequence):
    if i == 0:
        df_cumulative[kernel + '_cumulative'] = df_cumulative[kernel]
    else:
        df_cumulative[kernel + '_cumulative'] = df_cumulative[kernel_sequence[i-1] + '_cumulative'] + df_cumulative[kernel]

# Plotting
plt.figure(figsize=(10, 6))

# Collect line plots to access their colors for the fill
line_plots = []

# Plot lines for all kernels to store their colors
for kernel in kernel_sequence:
    cumulative_column = kernel + '_cumulative'
    line_plot, = plt.plot(df_cumulative['num_instances'], df_cumulative[cumulative_column], label=kernel)
    line_plots.append(line_plot)

plt.fill_between(df_cumulative['num_instances'], 0, df_cumulative[kernel_sequence[0] + '_cumulative'], color=line_plots[0].get_color(), alpha=0.3)

# Fill the area with the color of the line above
for i in range(len(line_plots)-1):
    plt.fill_between(df_cumulative['num_instances'], df_cumulative[kernel_sequence[i] + '_cumulative'], df_cumulative[kernel_sequence[i+1] + '_cumulative'], color=line_plots[i+1].get_color(), alpha=0.3)

plt.xlabel('Number of Instances')
plt.ylabel('Milliseconds')
plt.title('Cumulative Duration of Compute Kernels')
plt.legend()
plt.grid(True)
plt.show()

# Save the plot
plt.savefig('cumulative_duration.png')