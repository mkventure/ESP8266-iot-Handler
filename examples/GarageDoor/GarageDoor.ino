#include "config.h"

#include "IotHandler.h"
#include "Cover.h"
#include "BinarySensor.h"

IotHandler handler(WIFI_PIN, LED_PIN, WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);
BinarySensor_Pin reedSwitch(&handler, REED_SWITCH_PIN);
Cover garage(&handler, TOGGLE_PIN, [] {return reedSwitch.getState();});

void setup() {
  Serial.println("Start Setup");
  handler.set_onAction_callback(onAction);
  handler.set_onConnect_callback(onConnect);
}

void loop() {
  handler.loop();
  reedSwitch.loop();
  garage.loop();
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
  if (!reedSwitch.triggerAction(topicToProcess, payloadToProcess) and !garage.triggerAction(topicToProcess, payloadToProcess)) {
    Serial.println("Unrecognized action payload... taking no action!");
  }
}

void onConnect() {
  reedSwitch.onConnect();
  garage.onConnect();
}


