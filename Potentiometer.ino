
//อันนี้คือหมุนๆ
//หันด้านแบนเข้าตัว ซ้ายลบ ขวาบวก กลางA
#include <WiFi.h>          // ไลบรารีสำหรับ ESP32; ถ้าใช้ ESP8266 ให้ใช้ <ESP8266WiFi.h>
#include <MQTTClient.h>    // ไลบรารีสำหรับการเชื่อมต่อ MQTT

// ข้อมูลการเชื่อมต่อ Wi-Fi และ MQTT
const char* ssid = "Kantika";           // ชื่อ Wi-Fi ที่ต้องการเชื่อมต่อ
const char* password = "12345678";   // รหัสผ่าน Wi-Fi
const char* mqttServer = "phycom.it.kmitl.ac.th"; // เซิร์ฟเวอร์ MQTT
const int mqttPort = 1883;             // พอร์ตสำหรับการเชื่อมต่อ MQTT

// สร้างอ็อบเจกต์สำหรับการเชื่อมต่อ Wi-Fi และ MQTT
WiFiClient net;
MQTTClient client;

// กำหนดขาสำหรับ Potentiometer (ขา Analog)
const int potPin = A0; // ขา A0 สำหรับอ่านค่าจาก Potentiometer

// ฟังก์ชันที่ถูกเรียกเมื่อมีข้อความใหม่เข้ามาจาก MQTT
void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming message:");        // แสดงข้อความที่เข้ามา
  Serial.println("Topic: " + topic);          // แสดงหัวข้อของข้อความ
  Serial.println("Payload: " + payload);      // แสดงเนื้อหาของข้อความ
}

void setup() {
  Serial.begin(9600); // เริ่มต้น Serial สำหรับการสื่อสารที่ความเร็ว 9600 bps
  
  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);   // เริ่มการเชื่อมต่อกับ Wi-Fi
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
  while (!client.connect("arduinoClient")) {  // พยายามเชื่อมต่อ MQTT โดยใช้ client ID "arduinoClient"
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to MQTT broker!"); // แสดงข้อความเมื่อเชื่อมต่อสำเร็จ

  // สมัคร (Subscribe) หัวข้อที่ต้องการฟังข้อมูล
  client.subscribe("66070012/light");

  // ส่งข้อความไปยังหัวข้อ (เป็นตัวเลือก ไม่บังคับ)
  // client.publish("phycom/66070138", "Hello, MQTT from Arduino!");
}

void loop() {
  client.loop(); // ทำงานต่อเนื่องเพื่อรักษาการเชื่อมต่อกับ MQTT ไว้

  int potValue = analogRead(A0); // อ่านค่าจาก Potentiometer ที่ขา A0

  String num_str = String(potValue);   // แปลงค่าเป็น String
  char messageBuffer[10];
  num_str.toCharArray(messageBuffer, 10); // แปลง String เป็นอาร์เรย์ตัวอักษรเพื่อส่งผ่าน MQTT
  
  // ส่งข้อมูลไปยังหัวข้อทุก ๆ 2 วินาที
  client.publish("66070012/light", messageBuffer);
  Serial.println("Message published to 66070012/light : " + String(potValue)); // แสดงข้อความที่ส่งไปยัง MQTT

  delay(2000);   // หน่วงเวลา 2 วินาที (เพื่อการสาธิต)
}
