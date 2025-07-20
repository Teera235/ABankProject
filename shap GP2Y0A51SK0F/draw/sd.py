import cv2
import numpy as np
import pyautogui
import time
from PIL import Image

# ระบุพาธของภาพที่ต้องการโหลด
image_path = r"C:\Users\teerathap\Downloads\สกรีนช็อต 2025-07-04 155115.png"

# ตรวจสอบว่าไฟล์สามารถเปิดได้หรือไม่
try:
    img = Image.open(image_path)
    img.show()  # เปิดภาพใน viewer
    print("✅ Image loaded successfully with Pillow.")
except Exception as e:
    print(f"❌ Error loading image: {e}")
    exit()  # หากไม่สามารถโหลดได้ให้หยุดโปรแกรม

# แปลงภาพที่เปิดด้วย Pillow เป็น NumPy array สำหรับการประมวลผลด้วย OpenCV
img = np.array(img.convert('L'))  # แปลงภาพเป็น grayscale

# Resize ให้เล็กลงเพื่อความเร็ว
img = cv2.resize(img, (300, 300))

# ตรวจจับขอบ
edges = cv2.Canny(img, 100, 200)

# หาเส้น contour
contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

# สร้างภาพ preview
preview = np.zeros_like(img)
cv2.drawContours(preview, contours, -1, (255, 255, 255), 1)

# แสดง preview ให้ธีดู
cv2.imshow("🧠 Preview – ภาพที่จะวาด", preview)
print("🔎 กด Enter เพื่อยืนยันวาด | กด Q เพื่อยกเลิก")

while True:
    key = cv2.waitKey(100) & 0xFF  # รอ 100 ms รับ keycode
    if key == ord('q'):
        print("❌ ยกเลิกการวาด")
        cv2.destroyAllWindows()
        exit()
    elif key == 13:  # Enter key
        break

cv2.destroyAllWindows()
print("✅ ยืนยันแล้ว... เริ่มวาดใน 5 วินาที")
time.sleep(5)

# พิกัดซ้ายบนบนหน้าจอ (จุดเริ่มวาด)
start_x, start_y = 500, 300

# วาดเมาส์ตาม contour
for contour in contours:
    first = True
    for point in contour[::5]:  # ลดความละเอียดของจุดเพื่อเพิ่มความเร็วในการวาด
        x = start_x + point[0][0]
        y = start_y + point[0][1]
        if first:
            pyautogui.moveTo(x, y)
            pyautogui.mouseDown()
            first = False
        else:
            pyautogui.moveTo(x, y, duration=0.01)  # เพิ่มเวลาเล็กน้อยเพื่อให้การเคลื่อนที่นุ่มนวลขึ้น
    pyautogui.mouseUp()
