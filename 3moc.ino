// ---------------------------
// การต่อสายเซนเซอร์ต่างๆ
// - เซนเซอร์อุณหภูมิ: 
//   - ขาซ้าย ต่อ 5V
//   - ขากลาง ต่อ A1 (tempPin)
//   - ขาขวา ต่อ GND
//
// - Potentiometer:
//   - ขาซ้าย ต่อ GND
//   - ขากลาง ต่อ A0 (potPin)
//   - ขาขวา ต่อ 5V
//
// - เซนเซอร์อัลตร้าโซนิค:
//   - ขา Trig ต่อขา 9 (trigPin)
//   - ขา Echo ต่อขา 10 (echoPin)
//   - ขา VCC ต่อ 5V
//   - ขา GND ต่อ GND
// ---------------------------

#include <WiFi.h>          // ไลบรารีสำหรับ ESP32; ถ้าใช้ ESP8266 ให้ใช้ <ESP8266WiFi.h>
#include <MQTTClient.h>    // ไลบรารี MQTT สำหรับการสื่อสารกับ MQTT broker

// ข้อมูลการเชื่อมต่อ Wi-Fi และ MQTT
const char* ssid = "Kantika";            // ชื่อ Wi-Fi ที่ต้องการเชื่อมต่อ
const char* password = "12345678";       // รหัสผ่าน Wi-Fi
const char* mqttServer = "phycom.it.kmitl.ac.th"; // ที่อยู่เซิร์ฟเวอร์ MQTT
const int mqttPort = 1883;               // พอร์ตสำหรับการเชื่อมต่อ MQTT

// สร้างอ็อบเจกต์สำหรับการเชื่อมต่อ Wi-Fi และ MQTT
WiFiClient net;
MQTTClient client;

// กำหนดขาสำหรับเซนเซอร์ต่างๆ
const int tempPin = A1;      // ขา A1 สำหรับเซนเซอร์วัดอุณหภูมิ
const int potPin = A0;       // ขา A0 สำหรับ Potentiometer
const int trigPin = 9;       // ขา 9 สำหรับ Trig ของเซนเซอร์อัลตร้าโซนิค
const int echoPin = 10;      // ขา 10 สำหรับ Echo ของเซนเซอร์อัลตร้าโซนิค

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
  client.subscribe("66070012/temp");
  client.subscribe("66070012/light");
  client.subscribe("66070012/food");
}

void loop() {
  client.loop(); // รักษาการเชื่อมต่อกับ MQTT ให้ทำงานต่อเนื่อง

  // อ่านค่าอุณหภูมิจากเซนเซอร์ที่ขา A1
  int sensorValue = analogRead(tempPin);
  float voltage = sensorValue * (5.0 / 1023.0); // แปลงค่า analog เป็นค่าแรงดันไฟฟ้า โดยใช้ไฟอ้างอิง 5V
  float temperatureC = (voltage - 0.5) * 100;   // สำหรับ MCP9700 จะลบ 0.5V และคูณด้วย 100 เพื่อนำไปเป็นอุณหภูมิใน Celsius

  String temp_str = String(temperatureC); // แปลงค่าอุณหภูมิเป็น String
  char tempBuffer[10];
  temp_str.toCharArray(tempBuffer, 10);   // แปลง String เป็นอาร์เรย์ตัวอักษรเพื่อส่งผ่าน MQTT

  // ส่งข้อมูลอุณหภูมิไปยังหัวข้อ
  client.publish("66070012/temp", tempBuffer);
  Serial.println("Temperature published to 66070012/temp : " + temp_str); // แสดงข้อความที่ส่งไปยัง MQTT

  // อ่านค่า Potentiometer จากขา A0
  int potValue = analogRead(potPin);
  String pot_str = String(potValue);   // แปลงค่า Potentiometer เป็น String
  char potBuffer[10];
  pot_str.toCharArray(potBuffer, 10);  // แปลง String เป็นอาร์เรย์ตัวอักษรเพื่อส่งผ่าน MQTT

  // ส่งข้อมูล Potentiometer ไปยังหัวข้อ
  client.publish("66070012/light", potBuffer);
  Serial.println("Potentiometer published to 66070012/light : " + pot_str); // แสดงข้อความที่ส่งไปยัง MQTT

  // การวัดระยะทางด้วยเซนเซอร์อัลตร้าโซนิค
  digitalWrite(trigPin, LOW);  // ตั้งค่า Trig เป็น LOW
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH); // ตั้งค่า Trig เป็น HIGH เป็นเวลา 10 ไมโครวินาที
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // อ่านค่า Echo และคำนวณระยะทาง
  duration = pulseIn(echoPin, HIGH);
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
    num_str.toCharArray(messageBuffer, 10); // แปลงข้อความเป็นอาร์เรย์ตัวอักษรเพื่อส่งผ่าน MQTT
    
    // ส่งข้อความไปยังหัวข้อ
    client.publish("66070012/food", messageBuffer);
    Serial.println("Message published to 66070012/food : off");
  }
  else {
    String num_str = "on";
    char messageBuffer[10];
    num_str.toCharArray(messageBuffer, 10); // แปลงข้อความเป็นอาร์เรย์ตัวอักษรเพื่อส่งผ่าน MQTT
    
    // ส่งข้อความไปยังหัวข้อ
    client.publish("66070012/food", messageBuffer);
    Serial.println("Message published to 66070012/food : on");
  }

  delay(2000);   // หน่วงเวลา 2 วินาทีเพื่อการสาธิต
}
