import matplotlib.pyplot as plt
import pandas as pd

# Data provided by the user (10.0 cm - 15.0 cm)
data = {
    'Distance (cm)': [10.0, 10.5, 11.0, 11.5, 12.0, 12.5, 13.0, 13.5, 14.0, 14.5, 15.0],
    'Mean Voltage (V)': [0.624262, 0.602814, 0.581979, 0.563568, 0.544359, 0.519755, 0.499269, 0.478623, 0.456353, 0.441376, 0.436744],
    'Std Dev (V)': [0.006834, 0.007077, 0.007659, 0.005851, 0.007189, 0.006275, 0.006399, 0.007056, 0.006999, 0.007484, 0.006280],
    'Min (V)': [0.620, 0.596, 0.571, 0.557, 0.532, 0.513, 0.493, 0.474, 0.454, 0.430, 0.430],
    'Max (V)': [0.654, 0.635, 0.615, 0.591, 0.576, 0.552, 0.527, 0.508, 0.488, 0.479, 0.469]
}

df = pd.DataFrame(data)

# สร้างกราฟพร้อมปรับแต่งให้ดูเป็นมืออาชีพ
plt.figure(figsize=(10, 6)) # กำหนดขนาดของรูปให้เหมาะสม
plt.plot(
    df['Distance (cm)'],
    df['Mean Voltage (V)'],
    marker='o',          # ใช้จุดวงกลมสำหรับแต่ละข้อมูล
    linestyle='-',       # ใช้เส้นทึบเชื่อมจุด
    color='#d62728',     # เลือกใช้สีแดง (เพื่อความแตกต่างจากกราฟก่อนหน้า)
    linewidth=2.5,       # ทำให้เส้นหนาขึ้นเพื่อการมองเห็นที่ดีขึ้น
    markersize=8,        # ทำให้จุดข้อมูลใหญ่ขึ้น
    label='Mean Voltage (V)' # กำหนดป้ายชื่อสำหรับคำอธิบายกราฟ (legend)
)

# เพิ่มป้ายชื่อแกนและชื่อกราฟ พร้อมปรับขนาดตัวอักษรและรูปแบบ
plt.xlabel('Distance (cm)', fontsize=14, fontweight='bold')
plt.ylabel('Mean Voltage (V)', fontsize=14, fontweight='bold')
plt.title('Mean Voltage (V) Sensor Enclosed in Box (10 cm – 15 cm)', fontsize=16, fontweight='bold')

# ปรับแต่งตาราง Grid
plt.grid(True, linestyle='--', alpha=0.7) # ใช้เส้นประและโปร่งแสงเล็กน้อย

# ปรับแต่งตัวเลขบนแกน (Ticks)
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)

# กำหนดช่วงของแกน Y ให้เหมาะสมกับข้อมูล
# ค่าต่ำสุดของ Mean Voltage คือ ~0.436, สูงสุดคือ ~0.624
plt.ylim(0.40, 0.65) # กำหนดช่วงให้ชัดเจน

plt.legend(fontsize=12) # แสดงคำอธิบายกราฟ

plt.tight_layout() # ปรับเลย์เอาต์ของกราฟให้เหมาะสม ป้องกันข้อความทับซ้อน

# แสดงกราฟ (เมื่อคุณรันโค้ด)
plt.show()