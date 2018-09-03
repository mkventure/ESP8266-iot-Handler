#include "LightModule.h"

LightModule::LightModule(IotHandler* handler, int light_off_pin, int light_on_pin) {
  setupPins(light_off_pin, light_on_pin);
  setupHandler(handler);
  publish_light_status();
}

LightModule::LightModule(IotHandler* handler, int light_pin) {
  setupPins(light_pin, light_pin);
  setupHandler(handler);
  publish_light_status();
}

void LightModule::setupPins(int light_off_pin, int light_on_pin) {
  _light_off_pin = light_off_pin;
  _light_on_pin = light_on_pin;
  _light = STATE_OFF;

  pinMode(_light_off_pin, OUTPUT);
  pinMode(_light_on_pin, OUTPUT);
  // Set output pins LOW with an active-high relay
#ifdef ACTIVE_HIGH_RELAY==true
  digitalWrite(_light_off_pin, LOW);
  digitalWrite(_light_on_pin, LOW);
  // Set output pins HIGH with an active-low relay
#else
  digitalWrite(_light_off_pin, HIGH);
  digitalWrite(_light_on_pin, HIGH);
#endif

  //Set light to known off state
  if (light_off_pin != light_on_pin) {
    toggleRelay(light_off_pin);
  }
}


void LightModule::setupHandler(IotHandler* handler) {
  this->handler = handler;

  const char *light_action_topic_suffix = MQTT_LIGHT_ACTION_TOPIC_SUFFIX;
  strcpy(_mqtt_light_action_topic, handler->mqtt_clientId);
  strcat(_mqtt_light_action_topic, light_action_topic_suffix);

  const char *light_status_topic_suffix = MQTT_LIGHT_STATUS_TOPIC_SUFFIX;
  strcpy(_mqtt_light_status_topic, handler->mqtt_clientId);
  strcat(_mqtt_light_status_topic, light_status_topic_suffix);

  const char *light_command_topic_suffix = MQTT_LIGHT_COMMAND_TOPIC_SUFFIX;
  strcpy(_mqtt_light_command_topic, handler->mqtt_clientId);
  strcat(_mqtt_light_command_topic, light_command_topic_suffix);
}

void LightModule::toggleRelay(int pin) {
  //debug leds on while relay active
  digitalWrite(handler->led_pin, true); //red light off
  digitalWrite(handler->wifi_pin, false); //blue led on
#if(ACTIVE_HIGH_RELAY)
  digitalWrite(pin, HIGH);
  Serial.print("Toggled HIGH to Pin: ");
  Serial.println(pin);
  delay(RELAY_ACTIVE_TIME);
  digitalWrite(pin, LOW);
#else
  digitalWrite(pin, LOW);
  Serial.print("Toggled LOW to Pin: ");
  Serial.println(pin);
  delay(RELAY_ACTIVE_TIME);
  digitalWrite(pin, HIGH);
#endif
  //debug leds off
  digitalWrite(handler->wifi_pin, true); //blue led off
  digitalWrite(handler->led_pin, false);
}

void LightModule::publish_light_status() {
  Serial.print("Publishing Light State: ");

  if (_light == STATE_ON) {
    handler->client.publish(_mqtt_light_status_topic, LIGHT_ON_PAYLOAD, true);
    Serial.println(LIGHT_ON_PAYLOAD);
  }
  else if (_light == STATE_OFF) {
    handler->client.publish(_mqtt_light_status_topic, LIGHT_OFF_PAYLOAD, true);
    Serial.println(LIGHT_OFF_PAYLOAD);
  }
}

bool LightModule::triggerAction(String topic, String payload) {
  static unsigned long light_lastActionTime = 0;

  unsigned int currentTime = millis();

  if (topic == _mqtt_light_action_topic && payload == LIGHT_ON_PAYLOAD && currentTime - light_lastActionTime >= _actionTime && _light != STATE_ON) {
    light_lastActionTime = currentTime;
    toggleRelay(_light_on_pin);
    _light = STATE_ON;
    publish_light_status();
  }
  else if (topic == _mqtt_light_action_topic && payload == LIGHT_OFF_PAYLOAD && currentTime - light_lastActionTime >= _actionTime && _light != STATE_OFF) {
    light_lastActionTime = currentTime;
    toggleRelay(_light_off_pin);
    _light = STATE_OFF;
    publish_light_status();
  }
  else if (topic == _mqtt_light_command_topic && payload == LIGHT_INVERT_PAYLOAD && currentTime - light_lastActionTime >= _actionTime) {
    if (_light == STATE_OFF) {
      _light = STATE_ON;
    }
    else {
      _light = STATE_OFF;
    }
    publish_light_status();
  }
  else {
    return false;
  }
  return true;

}

void LightModule::onConnect() {
  Serial.print("Subscribing to ");
  Serial.print(_mqtt_light_action_topic);
  Serial.print(" & ");
  Serial.print(_mqtt_light_command_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_light_action_topic);
  handler->client.subscribe(_mqtt_light_command_topic);

  // Publish the current status on connect/reconnect
  publish_light_status();
}

void LightModule::loop() {

}

