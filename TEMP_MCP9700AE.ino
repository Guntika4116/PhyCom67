// วิธีต่อบอร์ด (ใช้ตัววัดอุณหภูมิสีดำสามขา หันด้านแบนเข้าหาตัว)
// 1. ขาซ้าย ต่อ 5V
// 2. ขากลาง ต่อ A0
// 3. ขาขวา ต่อ GND

//แก้ตรงที่มี *** นอกนั้นไม่ต้องแก้

#include <WiFi.h>
#include <PubSubClient.h>

const int tempPin = A0;  // ขากลาง

// ข้อมูลการเชื่อมต่อ WiFi
const char* ssid = "Kantika";       // ใส่ชื่อ WiFi Hotspot โทรศัพท์ ***
const char* password = "12345678";  // ใส่รหัสผ่าน WiFi *** 

// ข้อมูลการเชื่อมต่อ MQTT
const char* mqttServer = "phycom.it.kmitl.ac.th"; //ดู host จากหน้าเว็บช่องซ้ายสุด อาจจะ https://phycom.it.kmitl.ac.th/exam67
const int mqttPort = 1883;
const char* mqttClientID = "client_3e6d71c2"; //ใส่ client id อยู่ช่องขวาสุดในหน้าเว็บ ***
const char* topic = "66070012/temp"; //ใส่ชื่อ Topic ตามที่เขาให้มา Ex. 66070xxx/light ***

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(115200);
  
  // เชื่อมต่อ WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // ตั้งค่า MQTT server
  client.setServer(mqttServer, mqttPort);
}

void reconnect() {
  // พยายามเชื่อมต่อกับ MQTT broker
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(mqttClientID)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // อ่านค่าอนาล็อกจาก MCP9700AE
  int analogValue = analogRead(tempPin);
  float voltage = analogValue * (5.0 / 1023.0);     // แปลงค่าเป็นโวลต์
  float temperatureC = (voltage - 0.5) * 100;       // แปลงโวลต์เป็นอุณหภูมิ (เซลเซียส)

  // แปลงอุณหภูมิเป็นข้อความ
  char tempString[8];
  dtostrf(temperatureC, 1, 2, tempString);

  // ส่งค่าอุณหภูมิไปยัง MQTT topic
  Serial.print("Publishing temperature: ");
  Serial.println(tempString);
  client.publish(topic, tempString);

  delay(5000);  // ส่งข้อมูลทุกๆ 5 วินาที

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
