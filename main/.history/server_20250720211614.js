#include <WiFiS3.h>
#include <ArduinoHttpClient.h>

// ข้อมูล Wi-Fi
const char* ssid = "YOUR_SSID";      // ใส่ชื่อเครือข่าย Wi-Fi ของคุณ
const char* password = "YOUR_PASSWORD";  // ใส่รหัสผ่าน Wi-Fi ของคุณ

// เซิร์ฟเวอร์ URL
const char* server = "http://localhost";
const int port = 3000;
const char* resource = "/api/data";  // URL ของ API ที่เซิร์ฟเวอร์

WiFiClient wifi;  // การเชื่อมต่อ Wi-Fi
HttpClient client = HttpClient(wifi, server, port);  // ตั้งค่า HttpClient

void setup() {
  // เริ่มต้นการเชื่อมต่อ Serial
  Serial.begin(115200);
  
  // เชื่อมต่อ Wi-Fi
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // เริ่มต้นการเชื่อมต่อ HTTP
  sendData();
}

void loop() {
  // ส่งข้อมูลทุกๆ 5 วินาที
  sendData();
  delay(5000);  // ส่งข้อมูลทุก 5 วินาที
}

void sendData() {
  // ข้อมูล JSON ที่จะส่งไปยังเซิร์ฟเวอร์
  String jsonData = "{\"accelX\":100, \"accelY\":200, \"accelZ\":300, \"gyroX\":400, \"gyroY\":500, \"gyroZ\":600}";

  // ส่งข้อมูลผ่าน HTTP POST
  client.beginRequest();
  client.post(resource);
  client.sendHeader("Content-Type", "application/json");  // ระบุประเภทข้อมูลเป็น JSON
  client.sendHeader("Content-Length", jsonData.length());  // ขนาดของข้อมูล
  client.beginBody();
  client.print(jsonData);  // ส่งข้อมูล JSON ไปยังเซิร์ฟเวอร์
  client.endRequest();

  // ตรวจสอบการตอบกลับจากเซิร์ฟเวอร์
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status Code: ");
  Serial.println(statusCode);  // แสดงรหัสสถานะ HTTP
  Serial.print("Response: ");
  Serial.println(response);  // แสดงข้อความตอบกลับจากเซิร์ฟเวอร์
}
