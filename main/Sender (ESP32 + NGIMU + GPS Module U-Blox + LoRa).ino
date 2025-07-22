/******************************************************************************
 * SENDER (CUBESAT) CODE 
 *
 * MCU: ESP32
 * Sensors: NGIMU
 * Communication: LoRa Ra-01
 ******************************************************************************/

#include <SPI.h>
#include <NgimuReceive.h>
#include <Osc99.h>

#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

#define LORA_REG_OP_MODE 0x01
#define LORA_REG_FIFO 0x00
#define LORA_MODE_TX 0x03
#define LORA_MODE_RX 0x05

// Fast floating point variables
float pitch = 0.0;
float roll = 0.0;
float yaw = 0.0;

// Pre-allocated buffers
byte dataBuffer[12]; // 3 floats × 4 bytes each
volatile bool dataReady = false;

NgimuSensors ngimuSensors;
NgimuQuaternion ngimuQuaternion;
NgimuEuler ngimuEuler;

void setup() {
  Serial.begin(460800); // Increase baud rate
  Serial1.begin(460800); // Match IMU baud rate
  
  // Optimize SPI settings
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); // Fastest SPI
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  
  pinMode(LORA_SS, OUTPUT);
  pinMode(LORA_RST, OUTPUT);
  pinMode(LORA_DIO0, INPUT);
  
  digitalWrite(LORA_SS, HIGH); // Ensure SS is high initially
  
  // Fast reset sequence
  digitalWrite(LORA_RST, LOW);
  delayMicroseconds(100); // Use microseconds instead of milliseconds
  digitalWrite(LORA_RST, HIGH);
  delayMicroseconds(5000); // Minimum required delay
  
  LoRaSetMode(LORA_MODE_TX);
  
  NgimuReceiveInitialise();
  NgimuReceiveSetSensorsCallback(ngimuSensorsCallback);
  NgimuReceiveSetQuaternionCallback(ngimuQuaternionCallback);
  NgimuReceiveSetEulerCallback(ngimuEulerCallback);
}

void loop() {
  // Process multiple bytes at once if available
  int bytesAvailable = Serial1.available();
  if (bytesAvailable > 0) {
    for (int i = 0; i < bytesAvailable && i < 32; i++) { // Process max 32 bytes per loop
      NgimuReceiveProcessSerialByte(Serial1.read());
    }
  }
  
  // Only send if new data is available
  if (dataReady) {
    sendLoRaDataFast();
    dataReady = false;
  }
}

// Optimized LoRa transmission
void sendLoRaDataFast() {
  // Create local copies to avoid volatile issues
  float localPitch, localRoll, localYaw;
  
  // Copy all data to local variables (atomic operation)
  noInterrupts(); // Disable interrupts briefly
  localPitch = pitch;
  localRoll = roll;
  localYaw = yaw;
  interrupts();
  
  // Copy to buffer
  memcpy(&dataBuffer[0], &localPitch, 4);
  memcpy(&dataBuffer[4], &localRoll, 4);
  memcpy(&dataBuffer[8], &localYaw, 4);
  
  // Send all 12 bytes in one SPI transaction
  digitalWrite(LORA_SS, LOW);
  
  // Write to FIFO register efficiently
  SPI.transfer(LORA_REG_FIFO | 0x80);
  
  // Send all data in one burst
  for (int i = 0; i < 12; i++) {
    SPI.transfer(dataBuffer[i]);
  }
  
  digitalWrite(LORA_SS, HIGH);
  // Remove unnecessary delay
}

void ngimuSensorsCallback(const NgimuSensors ngimuSensorsData) {
  // Optional: Remove serial prints for maximum speed
  // Serial prints are very slow - comment out for production
  /*
  Serial.print("Gyroscope X: ");
  Serial.print(ngimuSensorsData.gyroscopeX);
  Serial.print(", Y: ");
  Serial.print(ngimuSensorsData.gyroscopeY);
  Serial.print(", Z: ");
  Serial.println(ngimuSensorsData.gyroscopeZ);
  */
}

void ngimuQuaternionCallback(const NgimuQuaternion ngimuQuaternionData) {
  // Comment out for maximum speed
  /*
  Serial.print("Quaternion w: ");
  Serial.print(ngimuQuaternionData.w);
  Serial.print(", x: ");
  Serial.print(ngimuQuaternionData.x);
  Serial.print(", y: ");
  Serial.print(ngimuQuaternionData.y);
  Serial.print(", z: ");
  Serial.println(ngimuQuaternionData.z);
  */
}

void ngimuEulerCallback(const NgimuEuler ngimuEulerData) {
  // Fast atomic update
  pitch = ngimuEulerData.pitch;
  roll = ngimuEulerData.roll;
  yaw = ngimuEulerData.yaw;
  dataReady = true; // Signal that new data is available
}

// Optimized LoRa write function
inline void LoRaWrite(uint8_t reg, uint8_t value) {
  digitalWrite(LORA_SS, LOW);
  SPI.transfer(reg | 0x80);
  SPI.transfer(value);
  digitalWrite(LORA_SS, HIGH);
}

void LoRaSetMode(uint8_t mode) {
  LoRaWrite(LORA_REG_OP_MODE, mode);
}

// Additional optimization: Use timer interrupt for precise timing
// Uncomment if you need precise transmission intervals
/*
#include <TimerOne.h>

void setupTimer() {
  Timer1.initialize(1000); // 1ms intervals
  Timer1.attachInterrupt(timerCallback);
}

void timerCallback() {
  if (dataReady) {
    sendLoRaDataFast();
    dataReady = false;
  }
}
*/
