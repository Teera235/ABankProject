# LoRa IMU System - Wiring & Latency Analysis

## 1. การต่อวงจร (Wiring Connections)

### Arduino/ESP32 กับ LoRa Module (SX1276/SX1278):
```
LoRa Module  →  Arduino Pin
VCC          →  3.3V
GND          →  GND
SCK          →  18 (SPI Clock)
MISO         →  19 (SPI MISO)
MOSI         →  23 (SPI MOSI)
NSS/CS       →  5  (LORA_SS)
RST          →  14 (LORA_RST)
DIO0         →  2  (LORA_DIO0)
```

### Arduino/ESP32 กับ N-IMU:
```
N-IMU        →  Arduino Pin
VCC          →  5V หรือ 3.3V
GND          →  GND
TX           →  16 (Serial1 RX)
RX           →  17 (Serial1 TX)
```

### Power Supply:
```
- Arduino/ESP32: 5V หรือ 3.3V
- LoRa Module: 3.3V (สำคัญ! ห้ามใช้ 5V)
- N-IMU: ตามที่ระบุในดาต้าชีต (มักเป็น 3.3V-5V)
```

## 2. การคำนวณ Latency

### ส่วนประกอบของ Latency:

#### A. IMU Sampling & Processing:
- **IMU Internal Processing**: 1-2ms
- **Serial Communication**: 
  ```
  Data size: ~50 bytes per packet
  Baud rate: 460,800 bps
  Time = (50 × 8) / 460,800 = 0.87ms
  ```

#### B. Arduino Processing:
- **Data Processing**: 0.1-0.5ms
- **Memory Copy**: 0.05ms
- **SPI Transfer (12 bytes)**:
  ```
  SPI Clock: 8MHz (SPI_CLOCK_DIV2)
  Time = (12 × 8) / 8,000,000 = 0.012ms
  ```

#### C. LoRa Transmission:
- **LoRa Parameters** (typical):
  ```
  Spreading Factor: SF7
  Bandwidth: 125kHz
  Coding Rate: 4/5
  Payload: 12 bytes
  ```
- **Air Time Calculation**:
  ```
  Preamble: 12.25 symbols
  Header: 8 symbols
  Payload: ceil((8×12-4×7+28+16)/(4×7))×4 = 8 symbols
  Total symbols: 12.25 + 8 + 8 = 28.25 symbols
  
  Symbol time = 2^7 / 125000 = 1.024ms
  Total air time = 28.25 × 1.024 = 28.9ms
  ```

#### D. LoRa Processing Overhead:
- **TX Setup**: 1-2ms
- **Buffer Processing**: 0.5ms

### **Total Latency Calculation:**

```
Component                    | Time (ms)
---------------------------- | ---------
IMU Sampling                 | 1.5
Serial Communication         | 0.87
Arduino Processing           | 0.6
LoRa Air Time               | 28.9
LoRa Overhead               | 2.0
---------------------------- | ---------
TOTAL LATENCY               | 33.87ms
```

## 3. การปรับปรุง Latency

### ลด Air Time:
```cpp
// ปรับพารามิเตอร์ LoRa สำหรับความเร็ว
Spreading Factor: SF7 → SF6  (ลด ~50% air time)
Bandwidth: 125kHz → 250kHz   (ลด ~50% air time)
```

### เพิ่ม Data Rate:
```
SF6 + 250kHz bandwidth:
Air time ≈ 7-10ms (ลดจาก 28.9ms)
```

### Optimized Latency:
```
Component                    | Optimized (ms)
---------------------------- | --------------
IMU Sampling                 | 1.0
Serial Communication         | 0.43 (921,600 bps)
Arduino Processing           | 0.3
LoRa Air Time               | 8.0 (SF6+250kHz)
LoRa Overhead               | 1.0
---------------------------- | --------------
OPTIMIZED TOTAL             | 10.73ms
```

## 4. Real-world Performance

### ปัจจัยที่ส่งผลต่อ Latency:
- **Distance**: ไม่ส่งผลต่อ latency (แต่ส่งผลต่อ reliability)
- **Interference**: อาจต้อง retransmit
- **Power Management**: sleep modes เพิ่ม latency
- **Processing Load**: อื่นๆ ที่ทำงานใน loop()

### Expected Performance:
```
Best Case (SF6, 250kHz):     ~11ms
Typical Case (SF7, 125kHz):  ~34ms
Worst Case (SF12, 125kHz):   ~200ms+
```

## 5. การทดสอบ Latency

### Code สำหรับวัด Latency:
```cpp
void ngimuEulerCallback(const NgimuEuler ngimuEulerData) {
  unsigned long timestamp = micros();
  
  pitch = ngimuEulerData.pitch;
  roll = ngimuEulerData.roll;
  yaw = ngimuEulerData.yaw;
  dataReady = true;
  
  Serial.print("IMU Data at: ");
  Serial.println(timestamp);
}

void sendLoRaDataFast() {
  unsigned long sendTime = micros();
  // ... send data ...
  Serial.print("LoRa sent at: ");
  Serial.println(sendTime);
}
```

### Performance Monitoring:
- วัดระยะเวลาจาก IMU callback ถึง LoRa transmission complete
- ใช้ oscilloscope วัด DIO0 pin เพื่อดูจุดเริ่มต้น/สิ้นสุด transmission
