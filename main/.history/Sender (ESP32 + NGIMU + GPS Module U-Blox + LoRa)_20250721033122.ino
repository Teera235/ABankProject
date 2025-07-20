#include <Wire.h>
#include <LoRa.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

const int csPin = 5;
const int resetPin = 14;
const int irqPin = 2;

MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial mySerial(1);

const int gpsRxPin = 16;
const int gpsTxPin = 17;

// LoRa setup
String message = "";
float ax, ay, az;
float gx, gy, gz;

void setup() {
  // Start Serial for debugging
  Serial.begin(115200);

  // Initialize LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  Serial.println("LoRa initialization successful");

  // Initialize MPU6050
  Wire.begin();
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  // Initialize GPS module
  mySerial.begin(9600, SERIAL_8N1, gpsRxPin, gpsTxPin);
}

void loop() {
  // Read data from MPU6050
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);

  // Get GPS data
  while (mySerial.available()) {
    gps.encode(mySerial.read());
  }

  // Prepare message to send
  message = "Pitch: " + String(ax) + ", Roll: " + String(ay) + ", Yaw: " + String(gz);
  message += ", Lat: " + String(gps.location.lat(), 6) + ", Lon: " + String(gps.location.lng(), 6);
  message += ", Alt: " + String(gps.altitude.meters()) + ", GPS Status: " + String(gps.status());

  // Send data over LoRa
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  // Print sent data to Serial Monitor for debugging
  Serial.println("Sent: " + message);

  delay(1000);  // Send data every 1 second
}
