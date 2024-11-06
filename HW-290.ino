#include <Arduino.h>
#include <string.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <PubSubClient.h>

// -----------------------------------------------------

#define WIFI_SSID "NEXT-18FO374"
#define WIFI_PASS "f1x6iz79"

#define MQTT_HOST "192.168.2.67"
#define MQTT_PORT 1883
#define MQTT_BASE_TOPIC "home/jardim/estufa/sensor_de_umidade_de_solo"

#define SENSOR_SAMPLES_COUNT 100
#define SECONDS_BETWEEN_MEASURES 60

// -----------------------------------------------------

#ifdef U8X8_HAVE_HW_SPI
  #include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
  #include <Wire.h>
#endif

#define SDA_PIN 5
#define SCL_PIN 6

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

const char* mqtt_host = MQTT_HOST;

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

WiFiClient espClient;
String IP_ESP = "xxx.xxx.xxx.xxx";

PubSubClient mqttClient(espClient);
String TOPICO_MQTT = MQTT_BASE_TOPIC;

float old_s1 = 0;
float old_s2 = 0;
float old_s3 = 0;
float old_s4 = 0;

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "LKHS_1";
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else { 
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup(void) { 
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  
  u8g2.begin();
  u8g2.setFont(u8g2_font_3x5im_te);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  u8g2.clearBuffer();
  u8g2.drawStr(2,8,"WiFi...");
  u8g2.sendBuffer();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  IPAddress ip = WiFi.localIP();
  IP_ESP = ip.toString();

  mqttClient.setServer(mqtt_host, MQTT_PORT);
}

void loop(void) {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  u8g2.clearBuffer();

  float sum_s1 = 0;
  float sum_s2 = 0;
  float sum_s3 = 0;
  float sum_s4 = 0;

  for(int i = 0; i < SENSOR_SAMPLES_COUNT; i++){
    float s1 = analogRead(0);
    float s2 = analogRead(1);
    float s3 = analogRead(3);
    float s4 = analogRead(4);

    sum_s1 += s1;
    sum_s2 += s2;
    sum_s3 += s3;
    sum_s4 += s4;

    delay(1);
  }

  sum_s1 = map(sum_s1 / SENSOR_SAMPLES_COUNT, 0, 4095, 0, 100);
  sum_s2 = map(sum_s2 / SENSOR_SAMPLES_COUNT, 0, 4095, 0, 100);
  sum_s3 = map(sum_s3 / SENSOR_SAMPLES_COUNT, 0, 4095, 0, 100);
  sum_s4 = map(sum_s4 / SENSOR_SAMPLES_COUNT, 0, 4095, 0, 100);

  char s1_str[8];
  char s2_str[8];
  char s3_str[8];
  char s4_str[8];
  
  dtostrf(sum_s1, 6, 0, s1_str);
  dtostrf(sum_s2, 6, 0, s2_str);
  dtostrf(sum_s3, 6, 0, s3_str);
  dtostrf(sum_s4, 6, 0, s4_str);

  Serial.print("s1   ");
  Serial.println(s1_str);

  Serial.print("s2   ");
  Serial.println(s2_str);

  Serial.print("s3   ");
  Serial.println(s3_str);

  Serial.print("s4   ");
  Serial.println(s4_str);
  
  Serial.println("-----------------------");

  u8g2.drawStr(2,5,IP_ESP.c_str());
  u8g2.drawStr(2,11,mqtt_host);

  if(sum_s1 != old_s1) {
    old_s1 = sum_s1;
    String message = TOPICO_MQTT + "/s1";
    mqttClient.publish(message.c_str(), s1_str);
  }

  if(sum_s2 != old_s2) {
    old_s2 = sum_s2;
    String message = TOPICO_MQTT + "/s2";
    mqttClient.publish(message.c_str(), s2_str);
  }

  if(sum_s3 != old_s3) {
    old_s3 = sum_s3;
    String message = TOPICO_MQTT + "/s3";
    mqttClient.publish(message.c_str(), s3_str);
  }

  if(sum_s4 != old_s4) {
    old_s4 = sum_s4;
    String message = TOPICO_MQTT + "/s4";
    mqttClient.publish(message.c_str(), s4_str);
  }

  u8g2.drawStr(2,20,s1_str);
  u8g2.drawStr(2,26,s2_str);
  u8g2.drawStr(30,20,s3_str);
  u8g2.drawStr(30,26,s4_str);
  u8g2.sendBuffer();
  
  delay(SECONDS_BETWEEN_MEASURES * 1000);
}
