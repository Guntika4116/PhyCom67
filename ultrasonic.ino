// ------------ อันนี้สองตา ---------------
// การต่อสายเซนเซอร์อัลตร้าโซนิค (Ultrasonic Sensor)
// - ขา Trig ของเซนเซอร์อัลตร้าโซนิค (Ultrasonic Sensor) ต่อกับขา 9 ของบอร์ด
// - ขา Echo ของเซนเซอร์อัลตร้าโซนิค ต่อกับขา 10 ของบอร์ด
// - ขา VCC ของเซนเซอร์ ต่อกับ 5V บนบอร์ด
// - ขา GND ของเซนเซอร์ ต่อกับ GND บนบอร์ด
// ---------------------------

#include <WiFi.h>          // ไลบรารี WiFi สำหรับบอร์ด ESP32; ถ้าใช้ ESP8266 ให้ใช้ <ESP8266WiFi.h>
#include <MQTTClient.h>    // ไลบรารี MQTT สำหรับการสื่อสารกับ MQTT broker

// ข้อมูลการเชื่อมต่อ Wi-Fi และ MQTT
const char* ssid = "Kantika";          // ชื่อ Wi-Fi ที่ต้องการเชื่อมต่อ
const char* password = "12345678";     // รหัสผ่าน Wi-Fi
const char* mqttServer = "phycom.it.kmitl.ac.th"; // ที่อยู่เซิร์ฟเวอร์ MQTT
const int mqttPort = 1883;             // พอร์ตสำหรับการเชื่อมต่อ MQTT

// สร้างออบเจกต์สำหรับการเชื่อมต่อ Wi-Fi และ MQTT
WiFiClient net;
MQTTClient client;

// กำหนดขา Trig และ Echo ของเซนเซอร์อัลตร้าโซนิค
const int trigPin = 9;
const int echoPin = 10;
long duration;               // ตัวแปรเก็บค่าระยะเวลา (เวลาในการเดินทางของคลื่นเสียง)
int distanceCm, distanceInch; // ตัวแปรเก็บระยะทางในหน่วยเซนติเมตรและนิ้ว

// ฟังก์ชันที่ถูกเรียกเมื่อมีข้อความใหม่เข้ามาจาก MQTT
void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming message:");
  Serial.println("Topic: " + topic);       // แสดงหัวข้อที่ได้รับข้อความ
  Serial.println("Payload: " + payload);   // แสดงเนื้อหาข้อความที่ได้รับ
}

void setup() {
  Serial.begin(9600);       // เริ่มต้นการสื่อสารแบบ Serial ที่ความเร็ว 9600 bps
  
  // ตั้งค่าโหมดขา Trig และ Echo สำหรับเซนเซอร์อัลตร้าโซนิค
  pinMode(trigPin, OUTPUT); // กำหนดให้ขา Trig เป็นขาออก
  pinMode(echoPin, INPUT);  // กำหนดให้ขา Echo เป็นขาเข้า

  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);           // เริ่มการเชื่อมต่อกับ Wi-Fi
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { // รอจนกว่าจะเชื่อมต่อสำเร็จ
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi"); // แสดงข้อความเมื่อเชื่อมต่อสำเร็จ

  // ตั้งค่า MQTT
  client.begin(mqttServer, mqttPort, net); // เริ่มต้นการเชื่อมต่อกับ MQTT broker
  client.onMessage(messageReceived);       // กำหนดให้เรียกใช้ฟังก์ชัน messageReceived เมื่อมีข้อความใหม่

  Serial.print("Connecting to MQTT...");
  while (!client.connect("arduinoClient")) {  // พยายามเชื่อมต่อกับ MQTT โดยใช้ client ID
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to MQTT broker!"); // แสดงข้อความเมื่อเชื่อมต่อ MQTT สำเร็จ

  // สมัครหัวข้อ (Subscribe) ที่ต้องการฟังข้อมูล
  client.subscribe("66070012/food");
}

void loop() {
  client.loop(); // รักษาการเชื่อมต่อกับ MQTT ให้ทำงานต่อเนื่อง

  // เตรียมส่งสัญญาณ Trig (ตั้งค่าเป็น LOW ก่อน)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // ตั้งค่า Trig เป็น HIGH เป็นเวลา 10 ไมโครวินาที
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // อ่านสัญญาณ Echo และเก็บเวลาเดินทางของคลื่นเสียงในหน่วยไมโครวินาที
  duration = pulseIn(echoPin, HIGH);

  // คำนวณระยะทางในหน่วยเซนติเมตรและนิ้ว
  distanceCm = duration * 0.034 / 2;    // คำนวณระยะทางเป็นเซนติเมตร
  distanceInch = duration * 0.0133 / 2; // คำนวณระยะทางเป็นนิ้ว

  // แสดงระยะทางใน Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // ถ้าระยะทางมากกว่า 20 ซม. ให้ส่งข้อความ "off" ผ่าน MQTT
  if (distanceCm > 20) {
    String num_str = "off";
    char messageBuffer[10];
    num_str.toCharArray(messageBuffer, 10); // แปลงข้อความเป็นอาร์เรย์ของตัวอักษรเพื่อส่งผ่าน MQTT
    
    // ส่งข้อความไปยังหัวข้อทุก ๆ 2 วินาที
    client.publish("66070012/food", messageBuffer);
    Serial.println("Message published to 66070012/food : off");
  }
  // ถ้าระยะทางน้อยกว่าหรือเท่ากับ 20 ซม. ให้ส่งข้อความ "on" ผ่าน MQTT
  else {
    String num_str = "on";
    char messageBuffer[10];
    num_str.toCharArray(messageBuffer, 10);
    
    // ส่งข้อความไปยังหัวข้อทุก ๆ 2 วินาที
    client.publish("66070012/food", messageBuffer);
    Serial.println("Message published to 66070012/food : on");
  }

  delay(2000);   // หน่วงเวลา 2 วินาที เพื่อการสาธิต
}
