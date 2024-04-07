import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('simulation_results_ci.csv')

error_rate = data['P/ErrorRate']
throughput_min, throughput_max = zip(*data['Throughput'].apply(eval))
latency_min, latency_max = zip(*data['Latency'].apply(eval))

throughput_mean = [(mini + maxi) / 2 for mini, maxi in zip(throughput_min, throughput_max)]
latency_mean = [(mini + maxi) / 2 for mini, maxi in zip(latency_min, latency_max)]

throughput_ci = [(maxi - mini) / 2 for mini, maxi in zip(throughput_min, throughput_max)]
latency_ci = [(maxi - mini) / 2 for mini, maxi in zip(latency_min, latency_max)]

plt.figure(figsize=(10, 6))

plt.subplot(2, 1, 1)
plt.plot(error_rate, throughput_mean, label='Throughput')
plt.fill_between(error_rate, [mean - ci for mean, ci in zip(throughput_mean, throughput_ci)],
                 [mean + ci for mean, ci in zip(throughput_mean, throughput_ci)], alpha=0.2)
plt.xlabel('Error Rate')
plt.ylabel('Throughput')
plt.title('Throughput vs Error Rate')
plt.legend()

plt.subplot(2, 1, 2)
plt.plot(error_rate, latency_mean, label='Latency')
plt.fill_between(error_rate, [mean - ci for mean, ci in zip(latency_mean, latency_ci)],
                 [mean + ci for mean, ci in zip(latency_mean, latency_ci)], alpha=0.2)
plt.xlabel('Error Rate')
plt.ylabel('Latency')
plt.title('Latency vs Error Rate')
plt.legend()

plt.tight_layout()
plt.show()
