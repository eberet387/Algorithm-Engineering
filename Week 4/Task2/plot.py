import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
from scipy.interpolate import make_interp_spline, BSpline

# Load the CSV file
print(os.getcwd())
data = pd.read_csv("output.csv")

# Extract the required columns
size = data['size'].to_numpy()  # 1st column: size
speedup_A = data['min_max_quicksort_speedup'].to_numpy()    # 3rd column: speedup_A
speedup_B = data['__gnu_parallel_speedup'].to_numpy()   # 5th column: speedup_B

# Plot the data
plt.figure(figsize=(10, 6))
plt.xscale("log")
plt.plot(size, speedup_A, label='Speedup min max quicksort', marker='o')
plt.plot(size, speedup_B, label='Speedup gnu parallel', marker='s')

# Add labels and title
plt.xlabel('size')
plt.ylabel('Speedup')
plt.title('Speedup over std:sort vs. Array Size')
plt.legend()

# Show the plot
plt.grid(True)

plt.savefig('speedup_plot.png', format='png', dpi=300)  # Adjust dpi for quality

plt.show()
