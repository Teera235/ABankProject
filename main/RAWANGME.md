CubeSat LoRa Communication System
โค้ดนี้ใช้สำหรับการสื่อสารข้อมูลเซ็นเซอร์ (pitch, roll, yaw) ระหว่าง Sender (ESP32 กับ NGIMU และ LoRa Ra-01/Ra-02) และ Receiver (ESP32 กับ LoRa Ra-01/Ra-02) ในระยะไม่เกิน 100 เมตร
ข้อควรระวัง
1. การตั้งค่าความถี่ LoRa

คำเตือน: Sender และ Receiver ต้องใช้ความถี่เดียวกัน (เช่น 433 MHz) มิฉะนั้นจะไม่สามารถสื่อสารได้
แนวทางแก้ไข:
ตั้งความถี่ใน setup():void setLoRaFrequency() {
  LoRaWrite(0x06, 0x6C); // 433 MHz
  LoRaWrite(0x07, 0x80);
  LoRaWrite(0x08, 0x00);
}


หรือใช้ LoRa.h:LoRa.begin(433E6);
LoRa.setSpreadingFactor(6);
LoRa.setSignalBandwidth(250E3);
LoRa.setCodingRate4(5);


ตรวจสอบกฎหมายความถี่ในประเทศไทย (433 MHz ใช้ได้ แต่ต้องไม่รบกวน)



2. คุณภาพเสาอากาศ

คำเตือน: Ra-02 ต้องบัดกรีเสาอากาศเอง (เช่น ลวดยาว 17.3 cm สำหรับ 433 MHz) ถ้าเสาอากาศไม่เหมาะสมจะทำให้สัญญาณอ่อนหรือสูญเสียแพ็กเก็ต
แนวทางแก้ไข:
ใช้เสาอากาศที่ออกแบบสำหรับ 433 MHz
สำหรับ Ra-01 ใช้ขั้วต่อ IPEX
ทดสอบ RSSI (> -80 dBm) และ SNR (> 0 dB) ในระยะ 100 เมตร



3. การจัดการพลังงาน

คำเตือน: การไม่ใช้ sleep mode ทำให้สิ้นเปลืองพลังงาน ซึ่งสำคัญใน CubeSat
แนวทางแก้ไข:
ใช้ sleep mode สำหรับ LoRa:#define LORA_MODE_SLEEP 0x00
void LoRaSleep() {
  LoRaWrite(LORA_REG_OP_MODE, LORA_MODE_SLEEP);
}


ใช้ deep sleep สำหรับ ESP32:esp_sleep_enable_timer_wakeup(1000000); // Sleep 1 วินาที
esp_deep_sleep_start();


ลดพลังงานส่ง:LoRaWrite(0x09, 0x80 | 0x0A); // 10 dBm





4. การจัดการข้อผิดพลาด

คำเตือน: โค้ดขาดการตรวจสอบข้อผิดพลาด (เช่น SPI ล้มเหลว, ข้อมูล NGIMU ไม่ถูกต้อง) อาจทำให้ระบบค้าง
แนวทางแก้ไข:
ตรวจสอบ LoRa module:uint8_t chipId = LoRaRead(0x42);
if (chipId != 0x12) {
  Serial.println("LoRa module not detected!");
  while (1);
}


ตรวจสอบข้อมูล NGIMU:if (isnan(ngimuEulerData.pitch) || isnan(ngimuEulerData.roll) || isnan(ngimuEulerData.yaw)) {
  Serial.println("Invalid IMU data!");
  return;
}


เพิ่ม timeout สำหรับ Serial:unsigned long startTime = micros();
while (Serial1.available() < 12 && micros() - startTime < 1000);





5. ความแม่นยำของ Packet Loss

คำเตือน: การคำนวณ Packet Loss% อาจคลาดเคลื่อนถ้า Sender ไม่ฝัง packet number
แนวทางแก้ไข:
ปรับ Sender ฝัง packet number (payload 16 bytes):uint32_t packetNum = 0;
void sendLoRaDataFast() {
  packetNum++;
  memcpy(&dataBuffer[12], &packetNum, 4);
  // ... ส่ง 16 bytes ...
}


ปรับ Receiver อ่าน packet number:uint32_t receivedPacketNum;
memcpy(&receivedPacketNum, &dataBuffer[12], 4);





6. ความร้อนและความเสถียร

คำเตือน: การทำงานต่อเนื่องอาจทำให้ Ra-01/Ra-02 หรือ ESP32 ร้อนเกินไป
แนวทางแก้ไข:
ใช้ watchdog timer:#include <esp_task_wdt.h>
void setup() {
  esp_task_wdt_init(5, true); // Reset ถ้าค้าง 5 วินาที
  esp_task_wdt_add(NULL);
}
void loop() {
  esp_task_wdt_reset();
}


ทดสอบอุณหภูมิในสภาพแวดล้อม 0-50°C



7. สัญญาณรบกวน

คำเตือน: สัญญาณ Wi-Fi, Bluetooth หรือสิ่งกีดขวางอาจทำให้ Packet Loss เพิ่มในระยะ 100 เมตร
แนวทางแก้ไข:
ใช้ SF6 และ BW=250 kHz เพื่อความเร็วและความน่าเชื่อถือ
เปิด CRC เพื่อตรวจสอบความสมบูรณ์ของข้อมูล
ทดสอบในสภาพแวดล้อมจริง



Latency

Typical (SF7, 125 kHz): ~33.87 ms (payload 12 bytes)
With Packet Number (16 bytes): ~46.83 ms
Optimized (SF6, 250 kHz): ~10.34 ms (12 bytes) หรือ ~13.39 ms (16 bytes)

การเชื่อมต่อ

LoRa Ra-01/Ra-02:
NSS → Pin 5, RESET → Pin 14, DIO0 → Pin 2
SPI: MOSI (Pin 23), MISO (Pin 19), SCK (Pin 18)
VCC → 3.3V, GND → GND


Ra-02: บัดกรีเสาอากาศ (17.3 cm สำหรับ 433 MHz)
ESP32: เชื่อมต่อ USB สำหรับ Serial Monitor (baud rate 460800 หรือ 921600)

คำแนะนำเพิ่มเติม

ใช้ LoRa.h เพื่อตั้งค่าความถี่, SF, BW ง่ายขึ้น
ทดสอบ RSSI/SNR ในระยะ 100 เมตร
ตรวจสอบ pinout ของ Ra-02 เพราะอาจต่างจาก Ra-01
