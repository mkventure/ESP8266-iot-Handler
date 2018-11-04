#include "config.h"

#include "IotHandler.h"
#include "IRTransceiver.h"

IotHandler handler(WIFI_PIN, LED_PIN, WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);
IRReceiver recv(&handler, RECEIVE_PIN);
IRTransmitter irsend(&handler, TRANSMIT_PIN);

void setup() {
  Serial.println("Start Setup");
  handler.set_onAction_callback(onAction);
  handler.set_onConnect_callback(onConnect);
  recv.setState(true);
}

void loop() {
  handler.loop();
  recv.loop();
  irsend.loop();
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
  if (!recv.triggerAction(topicToProcess, payloadToProcess) && !irsend.triggerAction(topicToProcess, payloadToProcess)) {
    Serial.println("Unrecognized action payload... taking no action!");
  }
}

void onConnect() {
  recv.onConnect();
  irsend.onConnect();
}



