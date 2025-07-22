/******************************************************************************
 * RECEIVER (GROUND STATION)
 *
 * MCU: ESP32
 * Communication: LoRa Ra-01
 ******************************************************************************/

#include <SPI.h>

#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

#define LORA_REG_OP_MODE 0x01
#define LORA_REG_FIFO 0x00
#define LORA_REG_RSSI_VALUE 0x1A
#define LORA_REG_PKT_SNR_VALUE 0x19
#define LORA_MODE_RX 0x05

// Buffer for receiving data
float pitch = 0.0;
float roll = 0.0;
float yaw = 0.0;
byte dataBuffer[12]; // 3 floats × 4 bytes each
volatile bool dataReceived = false;

// Packet tracking
uint32_t packetCount = 0;
uint32_t lastPacketNum = 0;
uint32_t missedPackets = 0;
float lossPercentage = 0.0;

void setup() {
  Serial.begin(460800); // High baud rate
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); // Fastest SPI
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  pinMode(LORA_SS, OUTPUT);
  pinMode(LORA_RST, OUTPUT);
  pinMode(LORA_DIO0, INPUT);
  
  digitalWrite(LORA_SS, HIGH); // Ensure SS is high

  // Fast reset sequence
  digitalWrite(LORA_RST, LOW);
  delayMicroseconds(100);
  digitalWrite(LORA_RST, HIGH);
  delayMicroseconds(5000);

  LoRaSetMode(LORA_MODE_RX); // Continuous receive mode

  // Attach interrupt for DIO0
  attachInterrupt(digitalPinToInterrupt(LORA_DIO0), onDataReceived, RISING);

  // Print header
  Serial.println("Format: Pitch, Roll, Yaw, RSSI, SNR, Packet#, Loss%");
}

void loop() {
  if (dataReceived) {
    processReceivedData();
    dataReceived = false;
  }
}

void onDataReceived() {
  // Read data from FIFO
  digitalWrite(LORA_SS, LOW);
  SPI.transfer(LORA_REG_FIFO | 0x00); // Read FIFO
  for (int i = 0; i < 12; i++) {
    dataBuffer[i] = SPI.transfer(0x00);
  }
  digitalWrite(LORA_SS, HIGH);
  dataReceived = true;
}

void processReceivedData() {
  // Atomic data extraction
  noInterrupts();
  memcpy(&pitch, &dataBuffer[0], 4);
  memcpy(&roll, &dataBuffer[4], 4);
  memcpy(&yaw, &dataBuffer[8], 4);
  interrupts();

  // Read RSSI and SNR
  int8_t rssi = -LoRaRead(LORA_REG_RSSI_VALUE); // RSSI in dBm (negative)
  int8_t snr = LoRaRead(LORA_REG_PKT_SNR_VALUE) / 4; // SNR in dB (scaled)

  // Update packet statistics
  packetCount++;
  // Assume Sender includes packet number in data (modify if needed)
  // For simplicity, using packetCount as Packet#
  if (packetCount > 1 && lastPacketNum + 1 != packetCount) {
    missedPackets += (packetCount - lastPacketNum - 1);
  }
  lastPacketNum = packetCount;
  lossPercentage = (missedPackets / (float)packetCount) * 100.0;

  // Print formatted output
  Serial.print(pitch);
  Serial.print(", ");
  Serial.print(roll);
  Serial.print(", ");
  Serial.print(yaw);
  Serial.print(", ");
  Serial.print(rssi);
  Serial.print(", ");
  Serial.print(snr);
  Serial.print(", ");
  Serial.print(packetCount);
  Serial.print(", ");
  Serial.println(lossPercentage);
}

inline uint8_t LoRaRead(uint8_t reg) {
  digitalWrite(LORA_SS, LOW);
  SPI.transfer(reg & 0x7F); // Read mode
  uint8_t value = SPI.transfer(0x00);
  digitalWrite(LORA_SS, HIGH);
  return value;
}

inline void LoRaWrite(uint8_t reg, uint8_t value) {
  digitalWrite(LORA_SS, LOW);
  SPI.transfer(reg | 0x80);
  SPI.transfer(value);
  digitalWrite(LORA_SS, HIGH);
}

void LoRaSetMode(uint8_t mode) {
  LoRaWrite(LORA_REG_OP_MODE, mode);
}
