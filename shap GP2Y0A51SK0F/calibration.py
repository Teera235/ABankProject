import serial
import time
import csv
import matplotlib.pyplot as plt
from collections import deque
import os

port = 'COM5'
baud = 9600
duration = 100 

folder = r"C:\Users\teerathap\Desktop\My Project\ABankProject\shap GP2Y0A51SK0F\raw(Box)"
os.makedirs(folder, exist_ok=True)
filename = os.path.join(folder, 'calibration_15cm.csv')

plt.ion()
fig, ax = plt.subplots()
times = deque(maxlen=200)
voltages = deque(maxlen=200)
line, = ax.plot([], [], label='Voltage (V)')
ax.set_title("Real-time Voltage")
ax.set_xlabel("Time (s)")
ax.set_ylabel("Voltage (V)")
ax.grid(True)
ax.legend()

ser = serial.Serial(port, baud)
time.sleep(2)
start_time = time.time()

with open(filename, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['Time (s)', 'Raw ADC', 'Voltage (V)'])

    while (time.time() - start_time) < duration:
        try:
            line_serial = ser.readline().decode().strip()
            parts = line_serial.split(',')
            if len(parts) == 3:
                t = float(parts[0]) / 1000.0
                raw = int(parts[1])
                voltage = float(parts[2])

                writer.writerow([t, raw, voltage])

                times.append(t)
                voltages.append(voltage)
                line.set_data(times, voltages)
                ax.set_xlim(max(0, t - 10), t + 1)
                ax.set_ylim(min(voltages) - 0.05, max(voltages) + 0.05)
                fig.canvas.draw()
                fig.canvas.flush_events()
        except Exception as e:
            print("Error:", e)
            continue

ser.close()
print(f"Data saved to:\n{filename}")
