#include "Cover.h"

Cover::Cover(IotHandler* handler, int openPin, int closePin, int stopPin,  MQTT_CALLBACK_GETSTATE_SIGNATURE, bool activeHighRelay, long relayActiveTime, const char* modName)
  : ToggleRelayModule(handler, modName, activeHighRelay, relayActiveTime)
{
  this->callbackGetState = callbackGetState;
  _setupPins(openPin, closePin, stopPin);
}
Cover::Cover(IotHandler* handler, int openPin, int closePin,  MQTT_CALLBACK_GETSTATE_SIGNATURE, bool activeHighRelay, long relayActiveTime, const char* modName)
  : ToggleRelayModule(handler, modName, activeHighRelay, relayActiveTime)
{
  this->callbackGetState = callbackGetState;
  _setupPins(openPin, closePin);
}
Cover::Cover(IotHandler* handler, int togglePin, MQTT_CALLBACK_GETSTATE_SIGNATURE, bool activeHighRelay, long relayActiveTime, const char* modName)
  : ToggleRelayModule(handler, modName, activeHighRelay, relayActiveTime)
{
  this->callbackGetState = callbackGetState;
  _setupPins(togglePin, togglePin);
}
void Cover::onConnect() {                                 //redefine onConnect
  ToggleRelayModule::onConnect();
  _publishStatus();
}

bool Cover::triggerAction(String topic, String payload) { //redefine triggerAction
  if (topic == _mqtt_action_topic && payload == _mqtt_cover_open_payload) {
    if (_coverOpenState == false) {
      Serial.println("Triggering OPEN relay!");
      _toggleRelay(_open_pin);
    }
  }
  else if (topic == _mqtt_action_topic && payload == _mqtt_cover_close_payload) {
    if (_coverOpenState == true) {
      Serial.println("Triggering CLOSE relay!");
      _toggleRelay(_close_pin);
    }
  }
  else if (topic == _mqtt_action_topic && payload == _mqtt_cover_stop_payload) {
    if (_enableStop) {
      Serial.println("Triggering STOP relay!");
      _toggleRelay(_stop_pin);
    }
    else {
      Serial.println("Publishing on-demand status update for Door1");
      setState(callbackGetState());
    }
  }
  else {
    //    Serial.println("Unrecognized action payload... taking no action!");
    return false;
  }
  return true;
}

void Cover::loop() {
  setState(callbackGetState());
}

void Cover::_setupPins(int openPin, int closePin, int stopPin) {
  _setupPins(openPin, closePin);

  _stop_pin = stopPin;
  pinMode(_stop_pin, OUTPUT);
  if (_activeHighRelay) {
    // Set output pins LOW with an active-high relay
    digitalWrite(_stop_pin, LOW);
  }
  else {
    // Set output pins HIGH with an active-low relay
    digitalWrite(_stop_pin, HIGH);
  }
  _enableStop = true;
}

void Cover::_setupPins(int openPin, int closePin) {
  _open_pin = openPin;
  _close_pin = closePin;
  pinMode(_open_pin, OUTPUT);
  pinMode(_close_pin, OUTPUT);
  if (_activeHighRelay) {
    // Set output pins LOW with an active-high relay
    digitalWrite(_open_pin, LOW);
    digitalWrite(_close_pin, LOW);
  }
  else {
    // Set output pins HIGH with an active-low relay
    digitalWrite(_open_pin, HIGH);
    digitalWrite(_close_pin, HIGH);
  }

  _enableStop = false;
}

bool Cover::setState(bool state) {
  if (state != _coverOpenState) {
    state = _coverOpenState;
    _publishStatus();
  }
}

void Cover::_publishStatus() {
  Serial.print("Publishing Cover State: ");
  if (_coverOpenState) {
    handler->client.publish(_mqtt_status_topic, _mqtt_cover_open_payload, true);
    Serial.println(_mqtt_cover_open_payload);
  }
  else {
    handler->client.publish(_mqtt_status_topic, _mqtt_cover_close_payload, true);
    Serial.println(_mqtt_cover_close_payload);
  }
}

