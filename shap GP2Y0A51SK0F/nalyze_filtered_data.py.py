import pandas as pd
import os
import re
from tkinter import filedialog, Tk

def analyze_voltage_csv():
    # เปิด Dialog ให้ธีเลือกหลายไฟล์ CSV
    root = Tk()
    root.withdraw()
    file_paths = filedialog.askopenfilenames(
        title="เลือกไฟล์ CSV ที่ต้องการวิเคราะห์",
        filetypes=[("CSV Files", "*.csv")]
    )

    if not file_paths:
        print("❌ ไม่ได้เลือกไฟล์ใดเลย")
        return pd.DataFrame()

    results = []

    # Pattern: ดึงค่า distance จากชื่อไฟล์ เช่น calibration_2.5cm_filtered.csv
    pattern = re.compile(r'(\d+(\.\d+)?)cm', re.IGNORECASE)

    for path in file_paths:
        filename = os.path.basename(path)
        match = pattern.search(filename)

        if not match:
            print(f"⚠️ ไม่พบระยะในชื่อไฟล์: {filename} (ข้าม)")
            continue

        try:
            distance_cm = float(match.group(1))
        except:
            print(f"⚠️ แปลงระยะจากชื่อไฟล์ไม่ได้: {filename} (ข้าม)")
            continue

        try:
            df = pd.read_csv(path)

            # ใช้ 'Voltage_Filtered' ถ้ามี ไม่งั้น fallback ไปใช้ 'Voltage (V)'
            voltage_col = 'Voltage_Filtered' if 'Voltage_Filtered' in df.columns else 'Voltage (V)'

            mean_v = df[voltage_col].mean()
            std_v = df[voltage_col].std()
            min_v = df[voltage_col].min()
            max_v = df[voltage_col].max()

            results.append({
                'File': filename,
                'Distance (cm)': distance_cm,
                'Mean Voltage (V)': round(mean_v, 6),
                'Std Dev (V)': round(std_v, 6),
                'Min (V)': round(min_v, 6),
                'Max (V)': round(max_v, 6)
            })

        except Exception as e:
            print(f"❌ Error ในไฟล์ {filename}: {e}")

    # รวมผลลัพธ์ทั้งหมด
    df_summary = pd.DataFrame(results)
    df_summary = df_summary.sort_values(by='Distance (cm)').reset_index(drop=True)

    # เซฟไฟล์สรุปในโฟลเดอร์เดียวกับไฟล์แรก
    if not df_summary.empty:
        output_path = os.path.join(os.path.dirname(file_paths[0]), 'voltage_summary_table.csv')
        df_summary.to_csv(output_path, index=False)
        print(f"\n✅ วิเคราะห์เสร็จสิ้น บันทึกที่: {output_path}")
        print(df_summary.to_string(index=False))
    else:
        print("❌ ไม่มีข้อมูลเพียงพอในการวิเคราะห์")

    return df_summary

# รันฟังก์ชัน
summary_df = analyze_voltage_csv()
