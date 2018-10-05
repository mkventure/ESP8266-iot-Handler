#include "CoverModule.h"

CoverModule::CoverModule(IotHandler* handler, MQTT_COVER_SENSE_SIGNATURE, int cover_close_pin, int cover_open_pin) {
  _setupPins(cover_close_pin, cover_open_pin);
  _setupHandler(handler);
  set_sense_callback(callbackSense);
}
CoverModule::CoverModule(IotHandler* handler, MQTT_COVER_SENSE_SIGNATURE, int cover_pin) {
  _setupPins(cover_pin, cover_pin);
  _setupHandler(handler);
  set_sense_callback(callbackSense);
}

CoverModule::CoverModule(IotHandler* handler, int cover_close_pin, int cover_open_pin) {
  _setupPins(cover_close_pin, cover_open_pin);
  _setupHandler(handler);
  set_sense_callback(NULL);
}
CoverModule::CoverModule(IotHandler* handler, int cover_pin) {
  _setupPins(cover_pin, cover_pin);
  _setupHandler(handler);
  set_sense_callback(NULL);
}

void CoverModule::set_sense_callback(MQTT_COVER_SENSE_SIGNATURE) {
  this->callbackSense = callbackSense;
  _cover = callbackSense();
}

void CoverModule::_setupPins(int cover_close_pin, int cover_open_pin) {
  _cover_close_pin = cover_close_pin;
  _cover_open_pin = cover_open_pin;

  pinMode(_cover_close_pin, OUTPUT);
  pinMode(_cover_open_pin, OUTPUT);

#ifdef ACTIVE_HIGH_RELAY==true
  digitalWrite(_cover_close_pin, LOW);
  digitalWrite(_cover_open_pin, LOW);
  // Set output pins HIGH with an active-low relay
#else
  digitalWrite(_cover_close_pin, HIGH);
  digitalWrite(_cover_open_pin, HIGH);
#endif
}

void CoverModule::_setupHandler(IotHandler* handler) {
  this->handler = handler;
  const char *cover_action_topic_suffix = MQTT_COVER_ACTION_TOPIC_SUFFIX;
  strcpy(_mqtt_cover_action_topic, handler->mqtt_clientId);
  strcat(_mqtt_cover_action_topic, cover_action_topic_suffix);

  const char *cover_status_topic_suffix = MQTT_COVER_STATUS_TOPIC_SUFFIX;
  strcpy(_mqtt_cover_status_topic, handler->mqtt_clientId);
  strcat(_mqtt_cover_status_topic, cover_status_topic_suffix);

  const char *cover_command_topic_suffix = MQTT_COVER_COMMAND_TOPIC_SUFFIX;
  strcpy(_mqtt_cover_command_topic, handler->mqtt_clientId);
  strcat(_mqtt_cover_command_topic, cover_command_topic_suffix);
}

void CoverModule::_publish_cover_status() {
  if (_cover ^ COVER_SWITCH_LOGIC) {
    //_cover false & COVER_SWITCH_LOGIC true OR
    //_cover true  & COVER_SWITCH_LOGIC false
    Serial.print("Publishing Cover State: ");
    Serial.print(_mqtt_cover_close_payload);    
    Serial.print(" to ");
    Serial.println(_mqtt_cover_status_topic);
    handler->client.publish(_mqtt_cover_status_topic, _mqtt_cover_close_payload, true);
  }
  else {
    //_cover false & COVER_SWITCH_LOGIC false OR
    //_cover true  & COVER_SWITCH_LOGIC true
    Serial.print("Publishing Cover State: ");
    Serial.print(_mqtt_cover_open_payload);    
    Serial.print(" to ");
    Serial.println(_mqtt_cover_status_topic);
    handler->client.publish(_mqtt_cover_status_topic, _mqtt_cover_open_payload, true);
  }
}


void CoverModule::_toggleRelay(int pin) {
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

void CoverModule::onConnect() {

  Serial.print("Subscribing to ");
  Serial.print(_mqtt_cover_action_topic);
  Serial.print(" & ");
  Serial.print(_mqtt_cover_command_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_cover_action_topic);
  handler->client.subscribe(_mqtt_cover_command_topic);

  // check sensor and publish the current status on connect/reconnect regardless of if sensor changed.
  if (!_checkSensor()) {
    _publish_cover_status();
  }
}

bool CoverModule::triggerAction(String topic, String payload) {
  static unsigned long _lastActionTime = 0;
  unsigned int currentTime = millis();

  if (topic == _mqtt_cover_action_topic && payload == _mqtt_cover_open_payload && currentTime - _lastActionTime >= _actionTime) {
    if (_cover ^ COVER_SWITCH_LOGIC) {
      Serial.println("Triggering Door1 OPEN relay!");
      _lastActionTime = currentTime;
      _toggleRelay(_cover_open_pin);
    }
  }
  else if (topic == _mqtt_cover_action_topic && payload == _mqtt_cover_close_payload && currentTime - _lastActionTime >= _actionTime) {
    if (!(_cover ^ COVER_SWITCH_LOGIC)) {
      Serial.println("Triggering Door 1 CLOSE relay!");
      _lastActionTime = currentTime;
      _toggleRelay(_cover_close_pin);
    }
  }
  else if (topic == _mqtt_cover_action_topic && payload == _mqtt_cover_stop_payload) {
    Serial.println("Publishing on-demand status update for Door1");
    _checkSensor();
  }
  else {
    //    Serial.println("Unrecognized action payload... taking no action!");
    return false;
  }
  return true;
}

//checks current sensor value returned by callbacksense
//stores value as _cover if changed
//returns true if changed, false if no change
bool CoverModule::_checkSensor() {
  bool val = callbackSense();
  if (_cover != val) {
    _cover = val;
    _publish_cover_status();
    return true;
  }
  return false;
}

void CoverModule::loop() {
  _checkSensor();
}


