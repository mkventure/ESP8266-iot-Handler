#include "Switch.h"

BinarySwitch::BinarySwitch(IotHandler* handler, const char* modName, bool state)
  : ActionModule(handler, modName)
{
  _switchState = state;
}

void BinarySwitch::onConnect() {                                 //redefine onConnect
  ActionModule::onConnect();
  _publishStatus();
}

void BinarySwitch::loop() {
ActionModule: loop();
  if (_timerFlag && millis() >= _timer) {
    setState(!_switchState);
  }
}

bool BinarySwitch::triggerAction(String topic, String payload) { //redefine triggerAction
  if (topic == _mqtt_action_topic && payload == _mqtt_switch_on_payload && _switchState != true) {
    setState(true);
  }
  else if (topic == _mqtt_action_topic && payload == _mqtt_switch_off_payload && _switchState != false) {
    setState(false);
  }
  else {
    return false;
  }
  return true;
}

bool BinarySwitch::setState(bool state) {
  if (_switchState != state) {
    _timerFlag = false;
    _switchState = state;
    _publishStatus();
    return true;
  }
  return false;
}

bool BinarySwitch::setStateFor(bool state, unsigned long timer) {
  if (_switchState != state) {
    _timerFlag = true;
    _switchState = state;
    _timer = millis() + timer;
    _publishStatus();
    return true;
  }
  return false;
}

void BinarySwitch::_publishStatus() {
  Serial.print("Publishing Switch State: ");
  if (_switchState) {
    handler->client.publish(_mqtt_status_topic, _mqtt_switch_on_payload, true);
    Serial.println(_mqtt_switch_on_payload);
  }
  else {
    handler->client.publish(_mqtt_status_topic, _mqtt_switch_off_payload, true);
    Serial.println(_mqtt_switch_off_payload);
  }
}

BinarySwitch_Pin::BinarySwitch_Pin(IotHandler* handler, int switch_pin, const char* modName, bool state)
  : BinarySwitch(handler, modName, state)
{
  _setupPins(switch_pin);
}

void BinarySwitch_Pin::_setupPins(int pin) {
  _switch_pin = pin;
  pinMode(_switch_pin, OUTPUT);
  digitalWrite(_switch_pin, _switchState);
}


bool BinarySwitch_Pin::setState(bool state) {
  if (BinarySwitch::setState(state)) {
    digitalWrite(_switch_pin, _switchState);
  }
}

bool BinarySwitch_Pin::setStateFor(bool state, unsigned long timer) {
  if (BinarySwitch::setStateFor(state, timer)) {
    digitalWrite(_switch_pin, _switchState);
  }
}

