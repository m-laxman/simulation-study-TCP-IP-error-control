import pandas as pd

df = pd.read_csv('simulation_results1.csv')


# # Calculate mean
# mean_throughput = 1.3172671
# mean_latency = 30.46004
#
# # Calculate standard deviation
# std_throughput = 2.6967646
# std_latency = 7.952754
#
# # Calculate variance
# var_throughput = 7.2725392
# var_latency = 63.246298

mean_throughput = df['Throughput'].mean()
mean_latency = df['Latency'].mean()

std_throughput = df['Throughput'].std()
std_latency = df['Latency'].std()

var_throughput = df['Throughput'].var()
var_latency = df['Latency'].var()

print(f"Throughput - Mean: {mean_throughput}, Standard Deviation: {std_throughput}, Variance: {var_throughput}")
print(f"Latency - Mean: {mean_latency}, Standard Deviation: {std_latency}, Variance: {var_latency}")


import numpy as np

t_value = 1.8331

se_throughput = std_throughput / np.sqrt(10)
se_latency = std_latency / np.sqrt(10)

ci_width_throughput = 2 * t_value * se_throughput
ci_width_latency = 2 * t_value * se_latency

ci_throughput = (mean_throughput - ci_width_throughput, mean_throughput + ci_width_throughput)
ci_latency = (mean_latency - ci_width_latency, mean_latency + ci_width_latency)

print(f"Throughput - Width of Confidence Interval: {ci_width_throughput}, Confidence Interval: {ci_throughput}")
print(f"Latency - Width of Confidence Interval: {ci_width_latency}, Confidence Interval: {ci_latency}")

