#include "light.h"

BinaryRelayLight::BinaryRelayLight(IotHandler* handler, int onPin, int offPin, bool activeHighRelay, long relayActiveTime, const char* modName)
  : ToggleRelayModule(handler, modName, activeHighRelay, relayActiveTime)
{
  _setupPins(onPin, offPin);
}
BinaryRelayLight::BinaryRelayLight(IotHandler* handler, int togglePin, bool activeHighRelay, long relayActiveTime, const char* modName)
  : ToggleRelayModule(handler, modName, activeHighRelay, relayActiveTime)
{
  _setupPins(togglePin, togglePin);
}

void BinaryRelayLight::onConnect() {                                 //redefine onConnect
  ToggleRelayModule::onConnect();
  _publishStatus();
}

bool BinaryRelayLight::triggerAction(String topic, String payload) { //redefine triggerAction
  if (topic == _mqtt_action_topic && payload == _mqtt_light_on_payload && _lightState != true) {
    ToggleRelayModule::_toggleRelay(_light_on_pin);
    _lightState = true;
    _publishStatus();
  }
  else if (topic == _mqtt_action_topic && payload == _mqtt_light_off_payload && _lightState != false) {
    ToggleRelayModule::_toggleRelay(_light_off_pin);
    _lightState = false;
    _publishStatus();
  }
  else if (topic == _mqtt_command_topic && payload == _mqtt_light_invert_payload && _isToggleLight()) {
    _lightState = !_lightState;
    _publishStatus();
  }
  else {
    return false;
  }
  return true;
}

void BinaryRelayLight::_setupPins(int onPin, int offPin) {
  _light_off_pin = offPin;
  _light_on_pin = onPin;
  pinMode(_light_off_pin, OUTPUT);
  pinMode(_light_on_pin, OUTPUT);
  if (_activeHighRelay) {
    // Set output pins LOW with an active-high relay
    digitalWrite(_light_off_pin, LOW);
    digitalWrite(_light_on_pin, LOW);
  }
  else {
    // Set output pins HIGH with an active-low relay
    digitalWrite(_light_off_pin, HIGH);
    digitalWrite(_light_on_pin, HIGH);
  }

  //Set light to known off state if there is specific off pin, otherwise just assume its off to start with.
  if (!_isToggleLight()) {
    ToggleRelayModule::_toggleRelay(_light_off_pin);
  }
  _lightState = false;
}

bool BinaryRelayLight::_isToggleLight() {
  return _light_off_pin == _light_on_pin;
}

void BinaryRelayLight::_publishStatus() {
  Serial.print("Publishing Light State: ");
  if (_lightState) {
    handler->client.publish(_mqtt_status_topic, _mqtt_light_on_payload, true);
    Serial.println(_mqtt_light_on_payload);
  }
  else {
    handler->client.publish(_mqtt_status_topic, _mqtt_light_off_payload, true);
    Serial.println(_mqtt_light_off_payload);
  }
}

