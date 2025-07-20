/******************************************************************************
 * SENDER (CUBESAT) CODE - FreeRTOS + LoRa
 *
 * MCU: ESP32
 * Sensors: NGIMU, U-blox GPS
 * Communication: LoRa Ra-01
 * Storage: MicroSD Card
 * Architecture: FreeRTOS with Fan-Out Queue Pattern
 ******************************************************************************/

// -- Libraries --
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <SD.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// -- Hardware Pins --
// LoRa Ra-01 Module
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_CS    5
#define LORA_RST   14
#define LORA_DIO0  2
// SD Card
#define SD_CS_PIN  15 // Use a different CS pin for SD
// UARTs
HardwareSerial GPSSerial(1); // RX: 9, TX: 10
HardwareSerial IMUSerial(2); // RX: 16, TX: 17

// -- LoRa Configuration --
#define LORA_FREQUENCY 915E6 // or 433E6, 868E6 based on your region/module

// -- Data Structure --
typedef struct DataPacket {
    float quaternion[4]; float accelerometer[3]; float gyroscope[3];
    double latitude; double longitude; double altitude; int satellites;
    unsigned long packetId;
} DataPacket;

// -- FreeRTOS Globals --
QueueHandle_t sensorQueue;
QueueHandle_t loraQueue;
QueueHandle_t sdQueue;
unsigned long packetCounter = 0;

// -- GPS Object --
TinyGPSPlus gps;

// =============================================================================
// Task 1: Sensor Reader Task
// Reads all sensors and places a complete data packet onto the central sensorQueue.
// =============================================================================
void sensorReaderTask(void *pvParameters) {
    Serial.println("Task: Sensor Reader... RUNNING");
    DataPacket packet;
    for (;;) {
        while (GPSSerial.available() > 0) gps.encode(GPSSerial.read());
        // TODO: Implement your actual NGIMU parsing logic here
        // if (IMUSerial.available()) { ... }

        if (gps.location.isUpdated() && gps.location.isValid()) {
            packet.latitude = gps.location.lat();
            packet.longitude = gps.location.lng();
            packet.altitude = gps.altitude.meters();
            packet.satellites = gps.satellites.value();

            // Placeholder for NGIMU data
            packet.quaternion[0] = 1.0;
            // ... fill other sensor data

            packet.packetId = packetCounter++;

            if (xQueueSend(sensorQueue, &packet, pdMS_TO_TICKS(10)) != pdPASS) {
                // Serial.println("Sensor queue full!");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Yield
    }
}

// =============================================================================
// Task 2: Distributor Task
// Takes a packet from sensorQueue and fans it out to loraQueue and sdQueue.
// =============================================================================
void distributorTask(void *pvParameters) {
    Serial.println("Task: Distributor... RUNNING");
    DataPacket packet;
    for (;;) {
        if (xQueueReceive(sensorQueue, &packet, portMAX_DELAY) == pdPASS) {
            // Send to LoRa queue
            xQueueSend(loraQueue, &packet, (TickType_t)0);
            // Send to SD logger queue
            xQueueSend(sdQueue, &packet, (TickType_t)0);
        }
    }
}

// =============================================================================
// Task 3: LoRa Sender Task
// Waits for data on the loraQueue and transmits it.
// =============================================================================
void loraSenderTask(void *pvParameters) {
    Serial.println("Task: LoRa Sender... RUNNING");
    DataPacket packet;
    for (;;) {
        if (xQueueReceive(loraQueue, &packet, portMAX_DELAY) == pdPASS) {
            Serial.printf("LoRa > Sending packet #%lu\n", packet.packetId);
            LoRa.beginPacket();
            LoRa.write((uint8_t *)&packet, sizeof(packet));
            LoRa.endPacket();
        }
    }
}

// =============================================================================
// Task 4: SD Logger Task
// Waits for data on the sdQueue and logs it to a file.
// =============================================================================
void sdLoggerTask(void *pvParameters) {
    Serial.println("Task: SD Logger... RUNNING");
    DataPacket packet;
    for (;;) {
        if (xQueueReceive(sdQueue, &packet, portMAX_DELAY) == pdPASS) {
            File dataFile = SD.open("/cubesat_log.csv", FILE_APPEND);
            if (dataFile) {
                dataFile.print(packet.packetId); dataFile.print(",");
                dataFile.print(millis()); dataFile.print(",");
                dataFile.print(packet.latitude, 6); dataFile.print(",");
                dataFile.print(packet.longitude, 6);
                dataFile.println();
                dataFile.close();
            } else {
                Serial.println("SD > Log failed!");
            }
        }
    }
}


void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("--- CUBESAT SENDER INITIALIZING ---");

    // Init UARTs
    GPSSerial.begin(9600, SERIAL_8N1, 9, 10);
    IMUSerial.begin(115200, SERIAL_8N1, 16, 17);

    // Init SPI
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);

    // Init SD Card
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card Mount Failed!");
    } else {
        File f = SD.open("/cubesat_log.csv", FILE_READ);
        if(!f) {
            f = SD.open("/cubesat_log.csv", FILE_WRITE);
            f.println("PacketID,Timestamp,Lat,Lon");
        }
        f.close();
        Serial.println("SD Card Initialized.");
    }

    // Init LoRa
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    Serial.printf("LoRa Initialized at %.0f MHz\n", LORA_FREQUENCY / 1E6);

    // Create FreeRTOS Queues
    sensorQueue = xQueueCreate(5, sizeof(DataPacket));
    loraQueue = xQueueCreate(5, sizeof(DataPacket));
    sdQueue = xQueueCreate(5, sizeof(DataPacket));

    // Create FreeRTOS Tasks
    xTaskCreatePinnedToCore(sensorReaderTask, "SensorReader", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(distributorTask, "Distributor", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(loraSenderTask, "LoRaSender", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(sdLoggerTask, "SDLogger", 4096, NULL, 1, NULL, 0);

    Serial.println("--- INITIALIZATION COMPLETE ---");
}

void loop() {
    vTaskDelete(NULL); // Loop is not used
}