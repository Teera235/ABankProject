#include <Wire.h>

// MPU6050 I2C address
#define MPU6050_ADDRESS 0x68

// Variables for MPU6050 data
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// For Quaternion function
float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
float axg, ayg, azg, gxrs, gyrs, gzrs;
float roll, pitch, yaw;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  // Initialize MPU6050
  writeByte(MPU6050_ADDRESS, 0x6B, 0x00); // Wake up MPU6050

  // Configure Gyro & Accelerometer
  writeByte(MPU6050_ADDRESS, 0x1B, 0x18); // Gyro FS = ±2000 degrees/s
  writeByte(MPU6050_ADDRESS, 0x1C, 0x10); // Accelerometer FS = ±8g

  pinMode(LED_BUILTIN, OUTPUT); // For activity indication
}

void loop() {
  readMPU6050();
  updateQuaternion();
  calculateOrientation();

  // Print roll, pitch, yaw to Serial Monitor
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print(", Pitch: ");
  Serial.print(pitch);
  Serial.print(", Yaw: ");
  Serial.println(yaw);

  // LED Blink to indicate activity
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle LED
  delay(100); // Adjust delay for desired performance
}

void readMPU6050() {
  uint8_t data[14];
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x3B); // Starting register for accelerometer
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDRESS, 14, true);

  // Read accelerometer and gyroscope data
  for (int i = 0; i < 7; i++) {
    int16_t* dataPointer = (i < 3) ? &accelX + i : &gyroX + (i - 3); // Handle data pointers
    *dataPointer = Wire.read() << 8 | Wire.read(); // Read 2 bytes per data point
  }
}

void updateQuaternion() {
  // Process accelerometer and gyroscope data
  axg = (float)accelX / 4096.0f;
  ayg = (float)accelY / 4096.0f;
  azg = (float)accelZ / 4096.0f;
  
  gxrs = (float)gyroX / 16.384f * 0.01745329; // Convert to radians
  gyrs = (float)gyroY / 16.384f * 0.01745329;
  gzrs = (float)gyroZ / 16.384f * 0.01745329;
}

void calculateOrientation() {
  // Use the sensor data to calculate roll, pitch, yaw
  yaw = atan2(2.0f * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.29577951;
  pitch = asin(2.0f * (q1 * q3 - q0 * q2)) * 57.29577951;
  roll = atan2(2.0f * (q0 * q1 + q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3) * 57.29577951;
}

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
  Wire.beginTransmission(address);
  Wire.write(subAddress);
  Wire.write(data);
  Wire.endTransmission();
}
