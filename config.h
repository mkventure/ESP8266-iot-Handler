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



/*----------------------------------------------- Module Config --------------------------------------------*/

#define ACTIVE_HIGH_RELAY true
#define ACTION_TIME 0
#define DEBOUNCE_TIME 2000
#define RELAY_ACTIVE_TIME 500

// Fan Parameters
#define MQTT_FAN_ACTION_TOPIC MQTT_CLIENTID "/fan/set"
#define MQTT_FAN_STATUS_TOPIC MQTT_CLIENTID "/fan/state"
#define MQTT_FANSPEED_ACTION_TOPIC MQTT_CLIENTID "/fan/speed/set"
#define MQTT_FANSPEED_STATUS_TOPIC MQTT_CLIENTID "/fan/speed/state"
#define FAN_ON_PAYLOAD "FAN_ON"
#define FAN_OFF_PAYLOAD "FAN_OFF"
#define FAN_LOW_PAYLOAD "FAN_LOW"
#define FAN_MED_PAYLOAD "FAN_MED"
#define FAN_HIGH_PAYLOAD "FAN_HIGH"
#define FAN_OFF_PIN 5
#define FAN_LOW_PIN 14
#define FAN_MED_PIN 12
#define FAN_HIGH_PIN 16
#define FAN_ON_DELAY 50


// Light Parameters
#define MQTT_LIGHT_ACTION_TOPIC MQTT_CLIENTID "/light/set"
#define MQTT_LIGHT_STATUS_TOPIC MQTT_CLIENTID "/light/state"
#define LIGHT_ON_PAYLOAD "LIGHT_ON"
#define LIGHT_OFF_PAYLOAD "LIGHT_OFF"
#define LIGHT_ON_PIN 13
#define LIGHT_OFF_PIN 13
//command topic

#define MQTT_COMMAND_ACTION_TOPIC MQTT_CLIENTID "/command"
