import matplotlib.pyplot as plt
import csv

print("Loading data and generating plots...")

# ==========================================
# Congestion Window(cwnd)_time plot
# ==========================================
times_cwnd = []
cwnd_values = []
with open('cwnd.csv', 'r') as f:
    reader = csv.reader(f)
    next(reader)
    for row in reader:
        times_cwnd.append(float(row[0]))
        cwnd_values.append(float(row[1]))

plt.figure(figsize=(10, 5))
plt.plot(times_cwnd, cwnd_values, drawstyle='steps-post', color='blue', linewidth=1.5)
plt.title('Congestion Window (cwnd) over Time')
plt.xlabel('Time (seconds)')
plt.ylabel('cwnd (Packets)')
plt.grid(True, linestyle='--', alpha=0.6)
plt.tight_layout()
plt.savefig('cwnd_plot.png', dpi=300)
print("[+] Chart 1 saved: cwnd_plot.png")

# ==========================================
# Throughput-time plot
# ==========================================
interval = 1.0
current_time_bound = interval
packets_in_interval = 0
last_ack = 0

times_tp = []
throughput_kbps = []

with open('events.log', 'r') as f:
    for line in f:
        if 'Received ACK expected seq_num' in line:
            parts = line.strip().split(' - EVENT: Received ACK expected seq_num ')
            if len(parts) == 2:
                t = float(parts[0])
                ack_num = int(parts[1])
                if ack_num > last_ack:
                    new_acked = ack_num - last_ack
                    last_ack = ack_num
                    
                    while t > current_time_bound:
                        times_tp.append(current_time_bound - (interval/2))
                        throughput_kbps.append((packets_in_interval * 1024) / 1024.0) 
                        current_time_bound += interval
                        packets_in_interval = 0
                        
                    packets_in_interval += new_acked

if packets_in_interval > 0:
    times_tp.append(current_time_bound - (interval/2))
    throughput_kbps.append((packets_in_interval * 1024) / 1024.0)

plt.figure(figsize=(10, 5))
plt.plot(times_tp, throughput_kbps, color='green', linewidth=1.5, marker='o', markersize=3)
plt.title('Throughput over Time')
plt.xlabel('Time (seconds)')
plt.ylabel('Throughput (KB/s)')
plt.grid(True, linestyle='--', alpha=0.6)
plt.tight_layout()
plt.savefig('throughput_plot.png', dpi=300)
print("[+] Chart 2 saved: throughput_plot.png")