#include "config.h"

#include "IotHandler.h"
#include "Fan.h"
#include "BinarySensor.h"
#include "Switch.h"
#include "Light.h"


void setup_j() {  
  pinMode(J1, OUTPUT);
  digitalWrite(J1, false);
  
  pinMode(J2, OUTPUT);
  digitalWrite(J2, false);
  
  pinMode(J3, OUTPUT);
  digitalWrite(J3, false);
  
  pinMode(J4, OUTPUT);
  digitalWrite(J4, false);
}

void stop_off() {
  digitalWrite(J1, false);
  digitalWrite(J2, false);
}
void stop_on() {
  digitalWrite(J1, true);  
  digitalWrite(J2, true);
}

void low_off() {
  digitalWrite(J1, false);
}
void low_on() {
  digitalWrite(J1, true);  
}

void med_off() {
  digitalWrite(J4, false);
}
void med_on() {
  digitalWrite(J4, true);  
}

void high_off() {
  digitalWrite(J2, false);
  digitalWrite(J4, false);
}
void high_on() {
  digitalWrite(J2, true);  
  digitalWrite(J4, true); 
}


IotHandler* handler = IotHandler::getInstance();
BinarySensor_Pin binarySensor1 (handler, "MBRPIR", PIR_PIN, "motion");

Fan_Function fan(handler, "MBRFan", 
                  stop_on, stop_off,
                  low_on, low_off,
                  med_on, med_off,
                  high_on, high_off, 
                  false, 400);

BinaryLight_Pin s1(handler, "RedLight", LED_BUILTIN, true, true);
BinaryLight_Pin s2(handler, "BlueLight", WIFI_PIN, true, true);
BinaryLight_Pin s3(handler, "FrontRedLight", RED_LIGHT_PIN, true, false);

void setup() {
  Serial.begin(115200);
  Serial.println("Begin Setup");
  setup_j();
  handler->setParams(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_PORT, MQTT_NODE_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_DISCOVERY_PREFIX);
  handler->setup();
  delay(50);
  fan.setState(false, FAN_MED);
}

void loop() {
  handler->loop();
}

