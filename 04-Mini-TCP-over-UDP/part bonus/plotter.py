import matplotlib.pyplot as plt
import csv

print("Loading data and generating Bonus plots...")

times_cwnd = []
cwnd_values = []
with open('cwnd.csv', 'r') as f:
    reader = csv.reader(f)
    next(reader) 
    for row in reader:
        times_cwnd.append(float(row[0]))
        cwnd_values.append(float(row[1]))

fast_rt_times = []
with open('events.log', 'r') as f:
    for line in f:
        if 'FAST RETRANSMIT' in line:
            parts = line.split(' - EVENT: FAST RETRANSMIT')
            fast_rt_times.append(float(parts[0]))

plt.figure(figsize=(10, 5))
plt.plot(times_cwnd, cwnd_values, drawstyle='steps-post', color='blue', linewidth=1.5, label='cwnd')

for fr_time in fast_rt_times:
    plt.axvline(x=fr_time, color='red', linestyle='--', alpha=0.7, linewidth=1.2)

if fast_rt_times:
    plt.plot([], [], color='red', linestyle='--', label='Fast Retransmit Triggered')

plt.title('Congestion Window with Fast Retransmit & Fast Recovery')
plt.xlabel('Time (seconds)')
plt.ylabel('cwnd (Packets)')
plt.grid(True, linestyle='--', alpha=0.6)
plt.legend()
plt.tight_layout()
plt.savefig('cwnd_bonus.png', dpi=300)
print("[+] Chart saved: cwnd_bonus.png")