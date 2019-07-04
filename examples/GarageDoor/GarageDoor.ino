#include "config.h"

#include "IotHandler.h"
#include "Switch.h"

IotHandler* handler = IotHandler::getInstance();

ToggleSwitch_Pin garage_toggle(handler, "GarageDoor", TOGGLE_PIN, 250);

void setup() {
  Serial.begin(115200);
  Serial.println("Begin Setup");
  handler->setParams(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_PORT, MQTT_NODE_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_DISCOVERY_PREFIX);
  handler->setup();
  delay(50);
}

void loop() {
  handler->loop();
}
