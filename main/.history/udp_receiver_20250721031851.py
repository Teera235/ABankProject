import serial
import time

# กำหนดพอร์ต Serial ที่จะเชื่อมต่อ (COM5)
serial_port = 'COM5'  # ปรับพอร์ตตามที่ใช้
baud_rate = 9600  # กำหนด Baud rate ที่ใช้

# เปิดการเชื่อมต่อ Serial
ser = serial.Serial(serial_port, baud_rate, timeout=1)  # ตั้งค่า timeout เป็น 1 วินาที

# สร้างตัวแปรสำหรับนับข้อมูล
num_samples = 0
start_time = time.time()  # เริ่มจับเวลา

# จำนวนข้อมูลที่ต้องการคำนวณ
duration = 5  # ใช้เวลา 5 วินาทีในการคำนวณ sampling rate

try:
    while True:
        # อ่านข้อมูลจาก serial port
        if ser.in_waiting > 0:  # ตรวจสอบว่าได้รับข้อมูลหรือไม่
            data = ser.readline()  # อ่านข้อมูลที่ได้รับ
            num_samples += 1  # นับจำนวนข้อมูลที่ได้รับ

        # คำนวณ Sampling Rate ทุกๆ 5 วินาที
        if time.time() - start_time >= duration:
            if num_samples > 0:  # ตรวจสอบว่าได้รับข้อมูลหรือไม่
                sampling_rate = num_samples / duration  # คำนวณ sampling rate
                print(f"Sampling Rate: {sampling_rate} samples per second")
            else:
                print("No data received during this period.")
            num_samples = 0  # รีเซ็ตการนับข้อมูล
            start_time = time.time()  # เริ่มจับเวลารอบใหม่
except KeyboardInterrupt:
    print("โปรแกรมถูกหยุดการทำงาน")

finally:
    # ปิดการเชื่อมต่อ Serial
    ser.close()
