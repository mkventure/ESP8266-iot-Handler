#include "config.h"
#include "iotHandler.h"

iotHandler iotHandler(WIFI_PIN, LED_PIN, WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

void setup() {
  iotHandler.set_onAction_callback(onAction);
  iotHandler.set_onConnect_callback(onConnect);
}


void loop() {
  
}

// Callback when MQTT message is received; passing topic and payload as parameters
void onAction(char* topic, byte* payload, unsigned int length) {
  
}

// Callback when MQTT is connected; 
void onConnect() {
  
}
