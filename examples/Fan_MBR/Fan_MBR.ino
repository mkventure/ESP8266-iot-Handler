#include "config.h"

#include "IotHandler.h"
#include "Fan.h"
#include "BinarySensor.h"

IotHandler* handler = IotHandler::getInstance();
BinarySensor_Pin binarySensor1 (handler, "MBRPIR", PIR_PIN, "motion");
Fan_Pin fan(handler, "MBRFan", FAN_OFF_PIN, FAN_LOW_PIN, FAN_MED_PIN, FAN_HIGH_PIN, false, 400);

void setup() {
  Serial.begin(115200);
  Serial.println("Begin Setup");
  handler->setParams(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_PORT, MQTT_NODE_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_DISCOVERY_PREFIX);
  handler->setup();
  delay(50);
  fan.setState(false, FAN_MED);
}

void loop() {
  handler->loop();
}
