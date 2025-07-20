import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import tkinter as tk
from tkinter import filedialog
import os

root = tk.Tk()
root.withdraw()  

folder_path = r"C:\Users\teerathap\Desktop\space\ABankProject\shap GP2Y0A51SK0F"
file_paths = filedialog.askopenfilenames(
    initialdir=folder_path,
    title="Select CSV Files",
    filetypes=[("CSV files", "*.csv")]
)

if not file_paths:
    print("No files selected. Exiting...")
    exit()

output_folder = r"C:\Users\teerathap\Desktop\space\ABankProject\graph"
os.makedirs(output_folder, exist_ok=True)

voltage_min = 0
voltage_max = 3.5  

for file_path in file_paths:
    try:
        df = pd.read_csv(file_path)

        mean = df['Voltage (V)'].mean()
        std = df['Voltage (V)'].std()

        plt.figure(figsize=(10, 4))
        plt.plot(df['Time (s)'], df['Voltage (V)'], label='Voltage (V)')
        plt.axhline(mean, color='red', linestyle='--', label=f'Mean = {mean:.3f} V')
        plt.fill_between(df['Time (s)'], mean - std, mean + std,
                         color='red', alpha=0.2, label=f'±1σ = {std:.3f} V')

        plt.title(f'Voltage Stability - {os.path.basename(file_path)}')
        plt.xlabel('Time (s)')
        plt.ylabel('Voltage (V)')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.ylim(voltage_min, voltage_max)  

        yticks = np.arange(voltage_min, voltage_max + 0.25, 0.25)
        plt.yticks(yticks)
        output_path = os.path.join(output_folder, f'plot_{os.path.basename(file_path).replace(".csv", ".png")}')
        plt.savefig(output_path)
        plt.close()

        print(f"Processed {os.path.basename(file_path)}: Mean = {mean:.3f} V, Std = {std:.3f} V")
        print(f"Saved graph to {output_path}")

    except Exception as e:
        print(f"Error processing {os.path.basename(file_path)}: {e}")

print("All selected files processed.")
