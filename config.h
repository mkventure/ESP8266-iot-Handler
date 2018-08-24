#include "secrets.h"

#define WIFI_PIN 2
#define LED_PIN LED_BUILTIN

// Wifi Parameters
#define WIFI_SSID SECRET_WIFI_SSID
#define WIFI_PASSWORD SECRET_WIFI_PASSWORD

// MQTT Parameters
#define MQTT_BROKER SECRET_MQTT_BROKER
#define MQTT_USERNAME SECRET_MQTT_USERNAME
#define MQTT_PASSWORD SECRET_MQTT_PASSWORD
#define MQTT_CLIENTID "livingroom_fan"

// Availibility
#define MQTT_AVAILIBILITY_TOPIC MQTT_CLIENTID "/availability"
#define AVAILABLE_PAYLOAD "online"
#define UNAVAILABLE_PAYLOAD "offline"

//OTA Settings
#define OTA_HOSTNAME "OTA_" MQTT_CLIENTID
