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
#define MQTT_NODE_ID "GARAGE"
#define MQTT_DISCOVERY_PREFIX "homeassistant"

// Module Config
#define TOGGLE_PIN 5
#define REED_SWITCH_PIN 4

/* YAML Configuration

- platform: mqtt
  name: GarageDoor
  state_topic: "GARAGE/BINARYSENSOR/STATE"
  payload_on: "BINARYSENSOR_ON"
  payload_off: "BINARYSENSOR_OFF"
  availability_topic: "GARAGE/AVAILIBILITY"
  payload_available: "ONLINE"
  payload_not_available: "OFFLINE"
  device_class: opening
  qos: 0

- platform: template
  sensors:
    garage_open30min:
      friendly_name: "Garage Open 30+ min"
      delay_on:
        minutes: 30
        seconds: 00
      value_template: "{{ is_state('cover.GarageDoor', 'open')}}"

- platform: mqtt
  name: GarageDoor
  state_topic: "GARAGE/COVER/STATE"
  command_topic: "GARAGE/COVER/SET"
  availability_topic: "GARAGE/AVAILIBILITY"
  qos: 0
  optimistic: false
  retain: false
  payload_open: "COVER_OPEN"
  payload_close: "COVER_CLOSE"
  payload_stop: "COVER_STOP"
  state_open: "COVER_OPEN"
  state_closed: "COVER_CLOSE"
  payload_available: "ONLINE"
  payload_not_available: "OFFLINE"

 */
