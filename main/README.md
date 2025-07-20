#  ESP32 LoRa Satellite Tracking System

ระบบติดตามดาวเทียมจำลองด้วย **ESP32**, **LoRa**, **GPS**, **NGIMU**, และ **SD Card**  
ส่งข้อมูลจาก CubeSat (Sender) ไปยัง Ground Station (Receiver) เพื่อบันทึกและประมวลผล

---

##  Bill of Materials (BOM)

| จำนวน | อุปกรณ์ | รุ่น/สเปกแนะนำ | หน้าที่ |
|-------|---------|----------------|---------|
| 2 | ESP32 Dev Board | ESP32-WROOM-32 | หน่วยประมวลผลหลัก |
| 2 | LoRa Module | Ra-01 (433MHz) หรือ Ra-02 (868/915MHz) | สื่อสารไร้สายระยะไกล |
| 2 | GPS Module | U-blox NEO-6M/8M | ตำแหน่งพิกัด |
| 1 | IMU (NGIMU) | Next Generation IMU | วัดทิศทาง/การเคลื่อนที่ |
| 2 | SD Card Module | SPI Interface | บันทึกข้อมูล |
| 2 | MicroSD Card | Class 10, 8GB+ | เก็บข้อมูล |
| 2 | LoRa Antenna | Spring/SMA | เสาสัญญาณ LoRa |
| - | Breadboard & Wires | Jumper Wires | ต่อวงจร |
| - | Power Supply | Micro USB/5V | จ่ายไฟ |

---

##   CubeSat (Sender) Wiring

| โมดูล | ขาโมดูล | ขา ESP32 | หมายเหตุ |
|-------|---------|----------|----------|
| **LoRa** | VCC | 3V3 | ห้ามต่อ 5V |
| | GND | GND | |
| | SCK | GPIO 18 | |
| | MISO | GPIO 19 | |
| | MOSI | GPIO 23 | |
| | NSS/CS | GPIO 5 | |
| | RST | GPIO 14 | |
| | DIO0 | GPIO 2 | |
| **SD Card** | VCC | 5V | มี Regulator |
| | GND | GND | |
| | SCK | GPIO 18 | แชร์กับ LoRa |
| | MISO | GPIO 19 | |
| | MOSI | GPIO 23 | |
| | CS | GPIO 15 | |
| **NGIMU** | VCC | 3V3/5V | ตรวจสอบคู่มือ |
| | GND | GND | |
| | TX | GPIO 16 | RX2 |
| | RX | GPIO 17 | TX2 |
| **GPS** | VCC | 3V3/5V | |
| | GND | GND | |
| | TX | GPIO 9 | RX1 |
| | RX | GPIO 10 | TX1 |

---

##  Ground Station (Receiver) Wiring

(เหมือนกับ CubeSat ยกเว้นไม่มี NGIMU)

---

##   Assembly Notes

- **SPI Bus**: LoRa & SD Card แชร์ SCK, MISO, MOSI แต่ใช้ CS แยก (LoRa: GPIO 5, SD: GPIO 15)
- **UART**:  
  - UART0 (GPIO 1, 3): อัปโหลดโค้ด/Serial Monitor  
  - UART1 (GPIO 9, 10): GPS  
  - UART2 (GPIO 16, 17): NGIMU
- **Voltage**:  
  - LoRa ใช้ 3.3V เท่านั้น  
  - SD Card/GPS ใช้ 3.3V หรือ 5V (ขึ้นกับรุ่น)
- **Antenna**:  
  - ติดตั้งเสาอากาศ LoRa ก่อนใช้งาน  
  - หลีกเลี่ยงการเปิด LoRa โดยไม่มีเสาอากาศ
- **GPS**:  
  - วางเสาอากาศในที่โล่งเพื่อรับสัญญาณ

---

## 🛠 ขั้นตอนการใช้งาน

1. **ต่อวงจร** ตามผังที่กำหนด
2. **อัปโหลดโค้ด** สำหรับ Sender และ Receiver
3. **ทดสอบ LoRa** สื่อสารระหว่าง CubeSat ↔ Ground Station
4. **ตรวจสอบการบันทึกข้อมูล** GPS/IMU ลง SD Card
5. **ดูข้อมูลเรียลไทม์** ผ่าน Serial Monitor

---

##  หมายเหตุ

- อ่านคู่มืออุปกรณ์แต่ละชิ้นก่อนใช้งานจริง
- ตรวจสอบแรงดันไฟให้เหมาะสมกับแต่ละโมดูล
- หากมีปัญหาในการเชื่อมต่อ ให้ตรวจสอบสายไฟและขา GPIO

---

**เอกสารนี้จัดทำเพื่อความสะดวกในการประกอบและใช้งานระบบ ESP32 LoRa Satellite Tracking System**  
**โปรดปฏิบัติตามขั้นตอนอย่างเคร่งครัดเพื่อความปลอดภัยและประสิทธิภาพสูงสุด**
