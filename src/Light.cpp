#import "Light.h"


BinaryLight::BinaryLight(IotHandler* handler, const char* mqtt_object_id, const char* mqtt_component)
  : ActionModule(handler, mqtt_object_id, mqtt_component), _lightState(false)
{
}

BinaryLight::BinaryLight(IotHandler* handler, const char* mqtt_object_id, bool state, const char* mqtt_component)
  : ActionModule(handler, mqtt_object_id, mqtt_component), _lightState(state)
{
}

bool BinaryLight::_handleAction(String topic, String payload) {
  bool flag = ActionModule::_handleAction(topic, payload);
  if (topic == _mqtt_command_topic && payload == _mqtt_state_on_payload && getState() != true) {
    setState(true);
    flag = true;
  }
  else if (topic == _mqtt_command_topic && payload == _mqtt_state_off_payload && getState() != false) {
    setState(false);
    flag = true;
  }
  return flag;
}

bool BinaryLight::setState(bool state) {
  if (getState() != state) {
    if (setHWState(state)) {
      _lightState = state;
      _publishStatus();
      return true;
    }
  }
  return false;
}

void BinaryLight::_publishStatus() {
  if (getState()) {
    Serial.printf("Publishing Light State: %s to %s\n", _mqtt_state_on_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_on_payload, true);
  }
  else {
    Serial.printf("Publishing Light State: %s to %s\n", _mqtt_state_off_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_off_payload, true);
  }
}


BinaryLight_TogglePin::BinaryLight_TogglePin(IotHandler* handler, const char* mqtt_object_id, int pin, bool initialState, bool state, unsigned long toggle_time, const char* mqtt_component)
  : BinaryLight(handler, mqtt_object_id, initialState, mqtt_component), lightControl(pin, state, toggle_time)
{}
BinaryLight_TogglePin::BinaryLight_TogglePin(IotHandler* handler, const char* mqtt_object_id, int pin, bool initialState, const char* mqtt_component)
  : BinaryLight(handler, mqtt_object_id, initialState, mqtt_component), lightControl(pin)
{}

bool BinaryLight_TogglePin::_handleAction(String topic, String payload) {
  bool flag = BinaryLight::_handleAction(topic, payload);
  if (topic == _mqtt_command_topic && payload == _mqtt_state_invert_payload) {
    //    _lightState = !_lightState;
    //    _publishStatus();
    Serial.printf("Publishing Invert State: %s to %s\n", _mqtt_state_invert_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_invert_payload, true);
    lightControl.toggle();
    flag = true;
  }
  return flag;
}
bool BinaryLight_TogglePin::setHWState(bool state) {
  return lightControl.toggle();
}



BinaryLight_Pin::BinaryLight_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, bool initialState, bool invertOutput, const char* mqtt_component)
  : BinaryLight(handler, mqtt_object_id, initialState, mqtt_component), lightControl(pin, initialState, invertOutput)
{}

bool BinaryLight_Pin::setHWState(bool state) {
  return lightControl.setState(state);
}
