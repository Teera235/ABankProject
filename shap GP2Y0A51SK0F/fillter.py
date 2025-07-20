import pandas as pd
import matplotlib.pyplot as plt
from scipy.signal import medfilt
import os
from tkinter import filedialog, Tk

def process_sensor_data(
    input_dir: str,
    output_csv_dir: str,
    output_graph_dir: str,
    voltage_col: str = 'Voltage (V)',
    time_col: str = 'Time (s)',
    kernel_size: int = 5
) -> None:
    os.makedirs(output_csv_dir, exist_ok=True)
    os.makedirs(output_graph_dir, exist_ok=True)
    if not os.path.isdir(input_dir):
        print(f"Error: The selected input directory does not exist: '{input_dir}'")
        return

    print("\n[DEBUG] Listing all files and folders found in the selected directory...")
    all_items_in_dir = os.listdir(input_dir)
    print(all_items_in_dir)
    print("[DEBUG] End of file list.\n")

    csv_files = [f for f in all_items_in_dir if f.lower().endswith('.csv')]

    if not csv_files:
        print(f"No CSV files found in '{input_dir}'. Please check file extensions.")
        print("Tip: In Windows File Explorer, go to 'View' and check the 'File name extensions' box.")
        return

    print(f"Starting to process {len(csv_files)} CSV files from: '{input_dir}'")

    for file_name in csv_files:
        file_path = os.path.join(input_dir, file_name)
        base_name = os.path.splitext(file_name)[0]
        print(f"\n--- Processing file: '{file_name}' ---")
        try:
            df = pd.read_csv(file_path)
            if voltage_col not in df.columns:
                print(f"  Error: Column '{voltage_col}' not found. Skipping.")
                continue
            if time_col not in df.columns:
                print(f"  Error: Column '{time_col}' not found. Skipping.")
                continue
            df['Voltage_Filtered'] = medfilt(df[voltage_col], kernel_size=kernel_size)
            print(f"  Median filter applied with kernel size: {kernel_size}")
            output_csv_path = os.path.join(output_csv_dir, f"{base_name}_filtered.csv")
            df.to_csv(output_csv_path, index=False)
            print(f"  Filtered data saved to: '{output_csv_path}'")
            plt.figure(figsize=(12, 6))
            plt.plot(df[time_col], df[voltage_col], label='Raw Voltage', alpha=0.6, linestyle=':')
            plt.plot(df[time_col], df['Voltage_Filtered'], label=f'Median Filtered (k={kernel_size})', linewidth=2.5)
            mean_voltage = df[voltage_col].mean()
            std_dev_voltage = df[voltage_col].std()
            plt.axhline(mean_voltage, color='red', linestyle='--', label=f'Mean = {mean_voltage:.3f} V')
            plt.fill_between(df[time_col], mean_voltage - std_dev_voltage, mean_voltage + std_dev_voltage, color='red', alpha=0.15, label=f'±1σ = {std_dev_voltage:.3f} V')
            plt.xlabel('Time (s)')
            plt.ylabel('Voltage (V)')
            plt.title(f'Voltage Stability - {file_name.replace(".csv", "")}')
            plt.legend()
            plt.grid(True, which='both', linestyle='--', linewidth=0.5)
            plt.ylim(0, 3.5)
            plt.tight_layout()
            output_graph_path = os.path.join(output_graph_dir, f"plot_{base_name}_filtered.png")
            plt.savefig(output_graph_path)
            print(f"  Comparison graph saved to: '{output_graph_path}'")
            plt.close()
        except Exception as e:
            print(f"  An error occurred while processing '{file_name}': {e}")
    print("\n--- All files processed. ---")


if __name__ == "__main__":
    root = Tk()
    root.withdraw()
    print("Please select the folder containing your RAW CSV files.")
    input_data_directory = filedialog.askdirectory(title="Select RAW Data Folder")

    if not input_data_directory:
        print("No folder selected. Exiting.")
    else:
        project_base_dir = os.path.dirname(input_data_directory)
        project_base_dir = os.path.dirname(project_base_dir)
        filtered_csv_output_directory = os.path.join(project_base_dir, "fillter", "csv")
        filtered_graph_output_directory = os.path.join(project_base_dir, "fillter", "graph")
        print(f"\nInput folder selected: {input_data_directory}")
        print(f"Filtered CSVs will be saved to: {filtered_csv_output_directory}")
        print(f"Graphs will be saved to: {filtered_graph_output_directory}\n")
        process_sensor_data(
            input_data_directory,
            filtered_csv_output_directory,
            filtered_graph_output_directory,
            kernel_size=7
        )