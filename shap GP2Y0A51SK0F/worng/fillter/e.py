import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from tkinter import filedialog, Tk
import os

from scipy.optimize import fsolve

def voltage_to_distance(V, a=5.421, b=0.841, c=0.113):
    return (a / (V - c))**(1/b)

Tk().withdraw()
files = filedialog.askopenfilenames(title="Select filtered CSV files", filetypes=[("CSV Files", "*.csv")])

results = []
for file in files:
    # Extract true distance from filename e.g. "calibration_10cm_filtered.csv"
    true_cm = float(os.path.basename(file).split('_')[1].replace('cm', ''))
    df = pd.read_csv(file)
    df = df.dropna(subset=['Voltage_Filtered'])
    
    est_dists = voltage_to_distance(df['Voltage_Filtered'])
    error = est_dists - true_cm

    results.append({
        'True Distance (cm)': true_cm,
        'Mean Sensor Distance (cm)': est_dists.mean(),
        'Absolute Error (cm)': np.abs(est_dists - true_cm).mean(),
        'Relative Error (%)': (np.abs(est_dists - true_cm).mean() / true_cm) * 100
    })

# Show results
df_result = pd.DataFrame(results).sort_values('True Distance (cm)')
print(df_result)

# Save summary
df_result.to_csv("error_analysis_summary.csv", index=False)
print("Saved to error_analysis_summary.csv")