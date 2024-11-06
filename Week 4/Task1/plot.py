import pandas as pd
import matplotlib.pyplot as plt
import os

# Load the CSV file
print(os.getcwd())
data = pd.read_csv("output.csv")

# Extract the required columns
num_threads = data['Num_Threads'].to_numpy()  # 1st column: num_threads
speedup_A = data['min_max_quicksort_speedup'].to_numpy()    # 3rd column: speedup_A
speedup_B = data['__gnu_parallel_speedup'].to_numpy()   # 5th column: speedup_B

# Plot the data
plt.figure(figsize=(10, 6))
plt.plot(num_threads, speedup_A, label='Speedup min max quicksort', marker='o')
plt.plot(num_threads, speedup_B, label='Speedup gnu parallel', marker='s')

# Add labels and title
plt.xlabel('Number of Threads')
plt.ylabel('Speedup')
plt.title('Speedup over std:sort vs. Number of Threads')
plt.legend()

# Show the plot
plt.grid(True)

plt.savefig('speedup_plot.png', format='png', dpi=300)  # Adjust dpi for quality

plt.show()
