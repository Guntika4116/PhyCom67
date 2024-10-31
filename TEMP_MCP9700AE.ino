// วิธีต่อบอร์ด (ใช้ตัววัดอุณหภูมิสีดำสามขา หันด้านแบนเข้าหาตัว)
// 1. ขาซ้าย ต่อ 5V
// 2. ขากลาง ต่อ A0
// 3. ขาขวา ต่อ GND

#include <WiFi.h>          // สำหรับ ESP32; ถ้าใช้ ESP8266 ให้ใช้ <ESP8266WiFi.h>
#include <MQTTClient.h>    // เรียกใช้ไลบรารี MQTT

// กำหนดข้อมูลการเชื่อมต่อ Wi-Fi และ MQTT
const char* ssid = "Kantika";            // ชื่อเครือข่าย Wi-Fi ที่จะเชื่อมต่อ
const char* password = "12345678";       // รหัสผ่าน Wi-Fi
const char* mqttServer = "phycom.it.kmitl.ac.th"; // ที่อยู่เซิร์ฟเวอร์ MQTT
const int mqttPort = 1883;               // พอร์ตสำหรับเชื่อมต่อ MQTT

WiFiClient net;            // สร้างอ็อบเจกต์ WiFiClient สำหรับการเชื่อมต่อ Wi-Fi
MQTTClient client;         // สร้างอ็อบเจกต์ MQTTClient สำหรับจัดการการเชื่อมต่อกับ MQTT broker

// ฟังก์ชันที่ถูกเรียกเมื่อมีข้อความใหม่เข้ามาจาก MQTT
void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming message:");
  Serial.println("Topic: " + topic);    // แสดงหัวข้อที่ได้รับข้อความ
  Serial.println("Payload: " + payload); // แสดงเนื้อหาข้อความที่ได้รับ
}

void setup() {
  Serial.begin(9600); // เริ่มต้นการสื่อสารแบบ Serial ที่ความเร็ว 9600 bps
  
  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);     // เริ่มการเชื่อมต่อ Wi-Fi
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { // รอจนกว่าจะเชื่อมต่อสำเร็จ
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi"); // แสดงข้อความเมื่อเชื่อมต่อสำเร็จ

  // ตั้งค่า MQTT
  client.begin(mqttServer, mqttPort, net); // เริ่มต้นการเชื่อมต่อ MQTT กับ server และ port ที่กำหนด
  client.onMessage(messageReceived);       // กำหนดให้เรียกใช้ฟังก์ชัน messageReceived เมื่อมีข้อความใหม่

  Serial.print("Connecting to MQTT...");
  while (!client.connect("arduinoClient")) {  // พยายามเชื่อมต่อ MQTT โดยใช้ client ID
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to MQTT broker!"); // แสดงข้อความเมื่อเชื่อมต่อ MQTT สำเร็จ

  // สมัคร (Subscribe) หัวข้อที่ต้องการฟังข้อมูล
  client.subscribe("66070012/temp");
}

void loop() {
  client.loop(); // ทำงานต่อเนื่องเพื่อรักษาการเชื่อมต่อ MQTT ไว้

  int sensorValue = analogRead(A0); // อ่านค่าจากเซนเซอร์ที่ขา A0
  float voltage = sensorValue * (5.0 / 1023.0); // แปลงค่า analog เป็นค่าแรงดันไฟฟ้า โดยใช้ไฟอ้างอิง 5V
  float temperatureC = (voltage - 0.5) * 100; // สำหรับ MCP9700 จะลบ 0.5V และคูณด้วย 100 เพื่อนำไปเป็นอุณหภูมิใน Celsius

  String num_str = String(temperatureC); // แปลงค่าอุณหภูมิเป็น String
  char messageBuffer[10];
  num_str.toCharArray(messageBuffer, 10); // แปลง String เป็นอาร์เรย์ตัวอักษรเพื่อส่งข้อความผ่าน MQTT
  
  // ส่งข้อมูลอุณหภูมิไปยังหัวข้อทุก ๆ 2 วินาที
  client.publish("66070012/temp", messageBuffer);
  Serial.println("Message published to 66070012/temp : " + String(temperatureC)); // แสดงข้อความที่ส่งไปยัง MQTT

  delay(2000);   // หน่วงเวลา 2 วินาที (สำหรับการสาธิต)
}


