#include "Switch.h"


Switch::Switch(IotHandler* handler, const char* mqtt_object_id, const char* mqtt_component)
  : ActionModule(handler, mqtt_object_id, mqtt_component)
{
}

bool Switch::_handleAction(String topic, String payload) { //redefine triggerAction
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

void Switch::_publishStatus() {
  if (getState()) {
    Serial.printf("Publishing Switch State: %s to %s\n", _mqtt_state_on_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_on_payload, true);
  }
  else {
    Serial.printf("Publishing Switch State: %s to %s\n", _mqtt_state_off_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_off_payload, true);
  }
}


bool Switch_Pin::setState(bool state) {
  if (switchPin.setState(state)) {
    _publishStatus();
    return true;
  }
  return false;
}
