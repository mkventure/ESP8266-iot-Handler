#include "config.h"

#include "IotHandler.h"
#include "FanModule.h"
#include "LightModule.h"

IotHandler handler(WIFI_PIN, LED_PIN, WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);
FanModule fan(&handler, FAN_OFF_PIN, FAN_LOW_PIN, FAN_MED_PIN, FAN_HIGH_PIN);
LightModule light(&handler, LIGHT_PIN);

void setup() {
  handler.set_onAction_callback(onAction);
  handler.set_onConnect_callback(onConnect);
}

void loop() {
  handler.loop();
  fan.loop();
  light.loop();
}

// Callback when MQTT message is received; passing topic and payload as parameters
void onAction(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();

  String topicToProcess = topic;
  payload[length] = '\0';
  String payloadToProcess = (char*)payload;
  if (!fan.triggerAction(topicToProcess, payloadToProcess) and !light.triggerAction(topicToProcess, payloadToProcess)) {
    Serial.println("Unrecognized action payload... taking no action!");
  }
}

void onConnect() {
  fan.onConnect();
  light.onConnect();
}


