import serial
import time

# ตั้งค่าพอร์ต COM5 และบอเรตเรตที่ 115200 (หรือค่าที่เหมาะสมกับอุปกรณ์ของคุณ)
ser = serial.Serial('COM5', 115200, timeout=1)

# รอให้การเชื่อมต่อเสร็จสมบูรณ์
time.sleep(2)

def read_data_from_serial():
    # อ่านข้อมูลจาก Serial port (อ่านแค่ 64 byte)
    data = ser.readline()
    return data

def process_data(data):
    # สมมติว่าได้รับข้อมูลในรูปแบบ CSV หรือ String เช่น "pitch, roll, yaw"
    data_str = data.decode('utf-8').strip()
    try:
        # แยกข้อมูลที่คั่นด้วย comma
        pitch, roll, yaw = map(float, data_str.split(','))
        print(f"Pitch: {pitch}, Roll: {roll}, Yaw: {yaw}")
        return pitch, roll, yaw
    except ValueError:
        print("Invalid data format")
        return None, None, None

def main():
    while True:
        # อ่านข้อมูลจาก Serial
        data = read_data_from_serial()
        if data:
            # ประมวลผลข้อมูลที่อ่านได้
            pitch, roll, yaw = process_data(data)
            
            if pitch is not None:
                # คำนวณหรือใช้ข้อมูลตามต้องการ
                print(f"Processed Data - Pitch: {pitch}, Roll: {roll}, Yaw: {yaw}")
        
        time.sleep(0.1)  # หน่วงเวลาเล็กน้อยเพื่อหลีกเลี่ยงการใช้ CPU มากเกินไป

if __name__ == "__main__":
    main()
