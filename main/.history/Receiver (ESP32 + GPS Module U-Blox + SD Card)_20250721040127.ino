#include <SPI.h>
#include <LoRa.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// LoRa Pin Configuration
const int csPin = 10;
const int resetPin = 9;
const int irqPin = 2;

// GPS Module setup
TinyGPSPlus gps;
HardwareSerial mySerial(1);
const int gpsRxPin = 16;
const int gpsTxPin = 17;

// SD card setup
File myFile;

// LoRa initialization and GPS setup
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

  // Initialize GPS module
  mySerial.begin(9600, SERIAL_8N1, gpsRxPin, gpsTxPin);

  // Initialize SD Card
  if (!SD.begin(4)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("SD Card initialization successful");

  // Open the log file
  myFile = SD.open("log.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("Logging started...");
  } else {
    Serial.println("Error opening file.");
  }

  // Create FreeRTOS task
  xTaskCreatePinnedToCore(receiveLoRaData, "Receive LoRa Data", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(readGPSData, "Read GPS", 2048, NULL, 1, NULL, 1);
}

void loop() {
  // FreeRTOS handles tasks, main loop stays empty
  vTaskDelay(portMAX_DELAY);  // Wait indefinitely
}

// Receive LoRa data task
void receiveLoRaData(void *pvParameters) {
  while (true) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String receivedData = "";
      while (LoRa.available()) {
        receivedData += (char)LoRa.read();
      }

      // Display received data
      Serial.print("Received: ");
      Serial.println(receivedData);

      // Save received data to SD Card
      if (myFile) {
        myFile.println(receivedData);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Check for data every 100 ms
  }
}

// Read GPS data task
void readGPSData(void *pvParameters) {
  while (true) {
    while (mySerial.available()) {
      gps.encode(mySerial.read());
    }

    if (gps.location.isUpdated()) {
      Serial.print("Latitude= ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= ");
      Serial.println(gps.location.lng(), 6);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Update GPS data every 500 ms
  }
}
