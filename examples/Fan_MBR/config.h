#include "secrets.h"

#define WIFI_PIN 2
#define LED_PIN LED_BUILTIN

// Wifi Parameters
#define WIFI_SSID SECRET_WIFI_SSID
#define WIFI_PASSWORD SECRET_WIFI_PASSWORD

// MQTT Parameters
#define MQTT_BROKER SECRET_MQTT_BROKER
#define MQTT_PORT 1883
#define MQTT_USERNAME SECRET_MQTT_USERNAME
#define MQTT_PASSWORD SECRET_MQTT_PASSWORD
#define MQTT_NODE_ID "MBR"
#define MQTT_DISCOVERY_PREFIX "homeassistant"

// Module Config
#define PIR_PIN 12     //D6

#define FAN_OFF_PIN 4 //D2
#define FAN_LOW_PIN 13 //D7
#define FAN_MED_PIN 14 //D5
#define FAN_HIGH_PIN 5 //D1


/*
From Silver:
Stop
High
GND
V+
GND
LOW
MED
*/