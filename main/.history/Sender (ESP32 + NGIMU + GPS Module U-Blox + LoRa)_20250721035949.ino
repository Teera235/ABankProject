/******************************************************************************
 * SENDER CODE (ESP32 + NGIMU + GPS + SD Card)
 * Reads sensor data, sends it via ESP-NOW, and logs it to an SD card.
 * Uses FreeRTOS for concurrent operations.
 ******************************************************************************/

// -- Libraries --
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// -- Hardware Configuration --
#define SD_CS_PIN 5 // Pin for SD Card Chip Select
HardwareSerial GPSSerial(1); // Use UART1 for GPS (RX: GPIO9, TX: GPIO10)
HardwareSerial IMUSerial(2); // Use UART2 for NGIMU (RX: GPIO16, TX: GPIO17)

// TODO: Replace with the MAC Address of the Receiver ESP32
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// -- Data Structure Definition --
typedef struct DataPacket {
    // NGIMU Data
    float quaternion[4];
    float accelerometer[3];
    float gyroscope[3];

    // GPS Data
    double latitude;
    double longitude;
    double altitude;
    int satellites;

    // System Data
    unsigned long timestamp;
} DataPacket;

// -- Global Variables --
DataPacket dataPacket;
esp_now_peer_info_t peerInfo;
QueueHandle_t dataQueue; // FreeRTOS Queue

// -- GPS Object --
TinyGPSPlus gps;

// -- Task Handles --
TaskHandle_t sensorReaderTaskHandle;
TaskHandle_t processingTaskHandle;

// =============================================================================
// Task 1: Sensor Reader Task
// - Reads GPS and IMU data continuously.
// - Packages data and sends it to a queue when both are fresh.
// =============================================================================
void sensorReaderTask(void *pvParameters) {
    Serial.println("Sensor Reader Task started on Core 1");
    
    // Dummy data for NGIMU simulation. Replace with actual parsing logic.
    float q_w = 1.0, q_x = 0.0, q_y = 0.0, q_z = 0.0;

    for (;;) {
        // 1. Read and parse GPS data from UART1
        while (GPSSerial.available() > 0) {
            gps.encode(GPSSerial.read());
        }

        // 2. Read and parse NGIMU data from UART2
        //    IMPORTANT: You need to implement the parsing logic specific to
        //    the NGIMU's serial output format. This is a placeholder.
        //    Example: if (IMUSerial.available()) { /* parse data */ }
        q_w += 0.001; // Fake data change for demonstration
        
        // 3. Check if we have a fresh and valid set of data
        if (gps.location.isUpdated() && gps.location.isValid()) {
            
            // Populate GPS data
            dataPacket.latitude = gps.location.lat();
            dataPacket.longitude = gps.location.lng();
            dataPacket.altitude = gps.altitude.meters();
            dataPacket.satellites = gps.satellites.value();

            // Populate IMU data (using placeholder data for now)
            dataPacket.quaternion[0] = q_w;
            dataPacket.quaternion[1] = q_x;
            dataPacket.quaternion[2] = q_y;
            dataPacket.quaternion[3] = q_z;
            // You would populate accelerometer and gyroscope similarly

            // Populate system data
            dataPacket.timestamp = millis();

            // Send the complete packet to the processing queue
            // The last parameter is the timeout. 0 means don't wait.
            if (xQueueSend(dataQueue, &dataPacket, (TickType_t)0) != pdPASS) {
                // Serial.println("Failed to send to queue. It might be full.");
            }
        }
        
        // Yield to other tasks. This delay is important for the scheduler.
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// =============================================================================
// Task 2: Processing and Forwarding Task
// - Waits for data from the queue.
// - Sends data via ESP-NOW.
// - Logs data to the SD card.
// =============================================================================
void processingAndForwardingTask(void *pvParameters) {
    Serial.println("Processing & Forwarding Task started on Core 0");
    DataPacket receivedPacket;

    for (;;) {
        // Wait indefinitely for a packet to arrive in the queue
        if (xQueueReceive(dataQueue, &receivedPacket, portMAX_DELAY) == pdPASS) {
            
            // 1. Send data via ESP-NOW
            esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&receivedPacket, sizeof(receivedPacket));
            if (result != ESP_OK) {
                Serial.println("Error sending data via ESP-NOW");
            }

            // 2. Log data to SD card
            File dataFile = SD.open("/datalog.csv", FILE_APPEND);
            if (dataFile) {
                dataFile.print(receivedPacket.timestamp);
                dataFile.print(",");
                dataFile.print(receivedPacket.latitude, 6);
                dataFile.print(",");
                dataFile.print(receivedPacket.longitude, 6);
                dataFile.print(",");
                dataFile.print(receivedPacket.altitude, 2);
                dataFile.print(",");
                // Add other data points as needed
                dataFile.println(receivedPacket.quaternion[0], 4); 
                dataFile.close();
            } else {
                Serial.println("Error opening datalog.csv on SD Card");
            }
        }
    }
}

// =============================================================================
// SETUP FUNCTION
// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("--- ESP32 Sender Initializing ---");

    // Initialize GPS Serial (UART1)
    GPSSerial.begin(9600); // Default U-blox baud rate

    // Initialize IMU Serial (UART2)
    IMUSerial.begin(115200); // Check NGIMU documentation for its baud rate

    // Initialize SD Card
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD Card Mount Failed!");
        // You might want to halt here if SD is critical
    } else {
        Serial.println("SD Card Initialized.");
        // Create header for CSV file if it doesn't exist
        File dataFile = SD.open("/datalog.csv", FILE_READ);
        if (!dataFile) {
            dataFile = SD.open("/datalog.csv", FILE_WRITE);
            dataFile.println("Timestamp,Lat,Lon,Alt,Q_W");
        }
        dataFile.close();
    }

    // Initialize ESP-NOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    Serial.println("ESP-NOW Initialized.");

    // Register peer
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // Create the FreeRTOS queue
    dataQueue = xQueueCreate(10, sizeof(DataPacket)); // Queue can hold 10 packets
    if (dataQueue == NULL) {
        Serial.println("Error creating the queue");
    }

    // Create FreeRTOS Tasks and pin them to specific cores
    xTaskCreatePinnedToCore(
        sensorReaderTask,         /* Task function. */
        "SensorReader",           /* name of task. */
        4096,                     /* Stack size of task */
        NULL,                     /* parameter of the task */
        1,                        /* priority of the task */
        &sensorReaderTaskHandle,  /* Task handle to keep track of created task */
        1);                       /* pin task to core 1 */

    xTaskCreatePinnedToCore(
        processingAndForwardingTask,
        "ProcessingForwarding",
        4096,
        NULL,
        1,
        &processingTaskHandle,
        0);                       /* pin task to core 0 */
        
    Serial.println("Initialization complete. Tasks are running.");
}

void loop() {
    // The loop is empty because all work is done in FreeRTOS tasks.
    vTaskDelete(NULL); // Delete the loop task
}