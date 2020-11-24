#include <ESP8266WiFi.h>
#include <HwinfoClient.h>

#define SERIAL_BAUD_RATE 115200
#define WIFI_SSID "MyWifiSSID"
#define WIFI_PASSWORD "MyWifiPassword"

#define HWINFO_SERVER "192.168.1.2"

void handleHwSensorReading(unsigned int group, unsigned int id, double value) {
  Serial.print("[0x");
  if(group < 16) Serial.print("0");
  Serial.print(group, HEX);
  Serial.print(" 0x");
  Serial.print(id, HEX);
  Serial.print("] = ");
  Serial.print(value);
  Serial.println();
}

HwinfoClient hwinfoClient(HWINFO_SERVER);
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connected!");

  hwinfoClient.setSensorReadHandler(handleHwSensorReading);
  hwinfoClient.connect();
}

void loop() {
  // put your main code here, to run repeatedly:

}
