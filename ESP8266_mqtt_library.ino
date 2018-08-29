#include "config.h"
#include "module_config.h"
#include "iotHandler.h"

iotHandler iotHandler(WIFI_PIN, LED_PIN, WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);


void setup() {
  setupModule();
  iotHandler.set_onAction_callback(onAction);
  iotHandler.set_onConnect_callback(onConnect);
}

void loop() {
  iotHandler.loop();
  loopModule();
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
  triggerAction(topicToProcess, payloadToProcess);
}
