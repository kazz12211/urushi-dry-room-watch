#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <HDC1000.h>
#include "SO1602A.h"
#include "config.h"


void callback(char *topic, byte* payload, unsigned int length) {
}

HDC1000 hdc1000;
SO1602A lcd(0x3c, 127);
WiFiClient wifiClient;
PubSubClient client((char *)mqttServer, mqttPort, callback, wifiClient);


void formattedFloat(float val, char buffer[]) {
  dtostrf(val, 4, 2, buffer);
}

void publishData() {
  char buff[50];
  char clientId[20];
  float h = hdc1000.getHumidity();
  float t = hdc1000.getTemperature();
  char hs[10];
  char ts[10];
  
  if(isnan(h) || isnan(t) || t == HDC1000_ERROR_CODE) {
    Serial.println("Failed to read from HDC1000 sensor");
    return;
  }
  sprintf(clientId, "esp_mqtt");
  if(!client.connected()) {
    client.connect(clientId, mqttUser, mqttPassword);
  }
  
  formattedFloat(h-8.0, hs);
  formattedFloat(t, ts);
  
  lcd.setCursor(0, 1);
  lcd.print(ts); lcd.print("C");
  lcd.setCursor(9, 1);
  lcd.print(hs); lcd.print("%");

  sprintf(buff, "{\"temperature\":%s, \"humidity\":%s}", ts, hs);
  if(client.connected()) {
    digitalWrite(led, 1);
  
    Serial.print("MQTT:Sending ");
    Serial.println(buff);
    int result = client.publish(topic, buff);
    if(result) {
      Serial.println("MQTT:Message sent");
    }
    client.disconnect();
    
    delay(300);
    digitalWrite(led, 0);
  }
}


void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);

  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  delay(500);
  
  lcd.begin(16, 2, LCD_5x8DOTS);
  delay(100);
  
  lcd.home();
  lcd.print("IP:");
  lcd.print(WiFi.localIP());

  delay(500);
  
  hdc1000.begin();
  
  delay(500);
  
}

void loop(void){
  publishData();
  delay(599500);
}
