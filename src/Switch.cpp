#include "Switch.h"

BinarySwitch::BinarySwitch(IotHandler* handler, int switch_pin, const char* modName, bool state)
  : ActionModule(handler, modName)
{
  _setupPins(switch_pin, state);
}

void BinarySwitch::onConnect() {                                 //redefine onConnect
  ActionModule::onConnect();
  _publishStatus();
}

void BinarySwitch::loop() {
  ActionModule:loop();
  if(_timerFlag && millis() >= _timer) {
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

void BinarySwitch::_setupPins(int pin, bool state) {
  _switch_pin = pin;
  pinMode(_switch_pin, OUTPUT);
  digitalWrite(_switch_pin, state);
  _switchState = state;
}

bool BinarySwitch::setState(bool state) {
  if (_switchState != state) {
    _timerFlag = false;
    _switchState = state;
    digitalWrite(_switch_pin, _switchState);
    _publishStatus();
    return true;
  }
  return false;
}

bool BinarySwitch::setStateFor(bool state, unsigned long timer) {  
  if (_switchState != state) {
    _timerFlag = true;
    _switchState = state;
    digitalWrite(_switch_pin, _switchState);
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

