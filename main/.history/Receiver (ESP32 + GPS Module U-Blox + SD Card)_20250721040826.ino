/******************************************************************************
 * RECEIVER (GROUND STATION) CODE - FreeRTOS + LoRa
 *
 * MCU: ESP32
 * Sensors: U-blox GPS
 * Communication: LoRa Ra-01
 * Storage: MicroSD Card
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
// UART
HardwareSerial GPSSerial(1); // RX: 9, TX: 10

// -- LoRa Configuration --
#define LORA_FREQUENCY 915E6 // Must match sender's frequency

// -- Data Structure (must match sender) --
typedef struct DataPacket {
    float quaternion[4]; float accelerometer[3]; float gyroscope[3];
    double latitude; double longitude; double altitude; int satellites;
    unsigned long packetId;
} DataPacket;

// -- FreeRTOS Globals --
QueueHandle_t receivedDataQueue;

// -- GPS Object --
TinyGPSPlus gps;

// =============================================================================
// Task 1: LoRa Receiver Task
// Listens for LoRa packets and puts them into a queue.
// =============================================================================
void loraReceiverTask(void *pvParameters) {
    Serial.println("Task: LoRa Receiver... RUNNING");
    DataPacket packet;
    for (;;) {
        int packetSize = LoRa.parsePacket();
        if (packetSize == sizeof(DataPacket)) {
            LoRa.readBytes((uint8_t *)&packet, packetSize);
            Serial.printf("LoRa < Received packet #%lu with RSSI %d\n", packet.packetId, LoRa.packetRssi());
            xQueueSend(receivedDataQueue, &packet, (TickType_t)0);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield
    }
}

// =============================================================================
// Task 2: Data Processing & Logging Task
// Takes packets from queue, prints to serial, and logs to SD card.
// =============================================================================
void dataProcessingTask(void *pvParameters) {
    Serial.println("Task: Data Processor... RUNNING");
    DataPacket packet;
    for (;;) {
        if (xQueueReceive(receivedDataQueue, &packet, portMAX_DELAY) == pdPASS) {
            // Print to serial
            Serial.printf("  Data: Lat=%.6f, Lon=%.6f\n", packet.latitude, packet.longitude);

            // Log to SD
            File dataFile = SD.open("/ground_station_log.csv", FILE_APPEND);
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

// =============================================================================
// Task 3: Ground Station's own GPS tracking task
// =============================================================================
void groundGpsTask(void* pvParameters){
    Serial.println("Task: Ground GPS... RUNNING");
    for(;;){
        while(GPSSerial.available() > 0) gps.encode(GPSSerial.read());

        if(gps.location.isUpdated()){
            File f = SD.open("/ground_gps.csv", FILE_APPEND);
            if(f){
                f.print(millis()); f.print(",");
                f.print(gps.location.lat(), 6); f.print(",");
                f.print(gps.location.lng(), 6); f.print(",");
                f.println(gps.satellites.value());
                f.close();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Log own position every second
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("--- GROUND STATION RECEIVER INITIALIZING ---");

    // Init UART
    GPSSerial.begin(9600, SERIAL_8N1, 9, 10);

    // Init SPI
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);

    // Init SD Card
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card Mount Failed!");
    } else {
        File f1 = SD.open("/ground_station_log.csv", FILE_READ);
        if(!f1) {
            f1 = SD.open("/ground_station_log.csv", FILE_WRITE);
            f1.println("PacketID,Timestamp,Lat,Lon");
        }
        f1.close();
        File f2 = SD.open("/ground_gps.csv", FILE_READ);
        if(!f2) {
            f2 = SD.open("/ground_gps.csv", FILE_WRITE);
            f2.println("Timestamp,Lat,Lon,Sats");
        }
        f2.close();
        Serial.println("SD Card Initialized.");
    }

    // Init LoRa
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    LoRa.receive(); // Set to receive mode
    Serial.printf("LoRa Initialized at %.0f MHz, waiting for packets...\n", LORA_FREQUENCY / 1E6);

    // Create FreeRTOS Queue
    receivedDataQueue = xQueueCreate(10, sizeof(DataPacket));

    // Create FreeRTOS Tasks
    xTaskCreatePinnedToCore(loraReceiverTask, "LoRaReceiver", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(dataProcessingTask, "DataProcessor", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(groundGpsTask, "GroundGPS", 4096, NULL, 1, NULL, 1);

    Serial.println("--- INITIALIZATION COMPLETE ---");
}

void loop() {
    vTaskDelete(NULL); // Loop is not used
}