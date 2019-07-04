#include "Fan.h"


Fan::Fan(IotHandler* handler, const char* mqtt_object_id, const char* mqtt_component)
  : ActionModule(handler, mqtt_object_id, mqtt_component)
{
  Serial.println("Start FanModule");
}

void Fan::_setup() {
  ActionModule::_setup();
  Serial.println("Setup FanModule");
  String s = "~/";
  s.concat(MQTT_FAN_SPEED_NAME);
  s.concat("/");
  s.concat(MQTT_COMMAND_TOPIC_SUFFIX);
  _setupTopic(_mqtt_fanspeed_command_topic, MQTT_FAN_SPEED_NAME, MQTT_COMMAND_TOPIC_SUFFIX);
  _addConfigElement("spd_cmd_t", s.c_str());
  String t = "~/";
  t.concat(MQTT_FAN_SPEED_NAME);
  t.concat("/");
  t.concat(MQTT_STATUS_TOPIC_SUFFIX);
  _setupTopic(_mqtt_fanspeed_status_topic, MQTT_FAN_SPEED_NAME, MQTT_STATUS_TOPIC_SUFFIX);
  _addConfigElement("spd_stat_t", t.c_str());
}

void Fan::_onConnect() {
  ActionModule::_onConnect();
  Serial.printf("Subscribing to %s\n", _mqtt_fanspeed_command_topic);
  getClient().subscribe(_mqtt_fanspeed_command_topic);
}

bool Fan::_handleAction(String topic, String payload) { //redefine triggerAction
  bool flag = ActionModule::_handleAction(topic, payload);

  if (topic == _mqtt_command_topic && payload == _mqtt_state_on_payload) {
    if (_fan_onDelay_bool) {
      flag = true;
      _delayOnFlag = true;
      _fan_onTime = millis() + _fan_onDelay;
      Serial.printf("Delayed On Setup... time:%d\tonTime:%d\n", millis(), _fan_onTime);
    }
    else {
      Serial.println("immediate on...\n");
      flag = setState(true, getSpeed());
    }
  }
  else if (topic == _mqtt_command_topic && payload == _mqtt_state_off_payload) {
    flag = setState(false, getSpeed());
  }
  else if (topic == _mqtt_fanspeed_command_topic && payload == _mqtt_fanSpeed_high_payload) {
    flag = setState(true, FAN_HIGH);
  }
  else if (topic == _mqtt_fanspeed_command_topic && payload == _mqtt_fanSpeed_med_payload) {
    flag = setState(true, FAN_MED);
  }
  else if (topic == _mqtt_fanspeed_command_topic && payload == _mqtt_fanSpeed_low_payload) {
    flag = setState(true, FAN_LOW);
  }
  return flag;
}

void Fan::_loop() {
  ActionModule::_loop();
  //The fan speed and state messages are received out of order. Only process the fan on commands if speed is not already affected.

  if (_fan_onDelay_bool && _delayOnFlag && millis() >= _fan_onTime) {
    Serial.printf("Delayed On Start... time:%d\tonTime%d\t\n", millis(), _fan_onTime);
    _delayOnFlag = false;
    setState(true, getSpeed());
  }
}


void Fan::_publishStatus() {
  _publishFanStatus();
  _publishSpeedStatus();
}

void Fan::_publishFanStatus() {
  if (getState()) {
    Serial.printf("Publishing Fan State: %s to %s\n", _mqtt_state_on_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_on_payload, true);
  }
  else {
    Serial.printf("Publishing Fan State: %s to %s\n", _mqtt_state_off_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_off_payload, true);
  }
}

void Fan::_publishSpeedStatus() {
  switch (_fanSpeed) {
    case FAN_LOW:
      Serial.printf("Publishing Fan Speed: %s to %s\n", _mqtt_fanSpeed_low_payload, _mqtt_fanspeed_status_topic);
      getClient().publish(_mqtt_fanspeed_status_topic, _mqtt_fanSpeed_low_payload, true);
      break;
    case FAN_MED:
      Serial.printf("Publishing Fan Speed: %s to %s\n", _mqtt_fanSpeed_med_payload, _mqtt_fanspeed_status_topic);
      getClient().publish(_mqtt_fanspeed_status_topic, _mqtt_fanSpeed_med_payload, true);
      break;
    case FAN_HIGH:
      Serial.printf("Publishing Fan Speed: %s to %s\n", _mqtt_fanSpeed_high_payload, _mqtt_fanspeed_status_topic);
      getClient().publish(_mqtt_fanspeed_status_topic, _mqtt_fanSpeed_high_payload, true);
      break;
  }
}

bool Fan::setState(bool state, FanSpeed speed) {
  if (state) {
    bool success = false;
    if (speed == FAN_HIGH) {
      success = _setHigh();
    }
    else if (speed == FAN_MED) {
      success = _setMed();
    }
    else if (speed == FAN_LOW) {
      success = _setLow();
    }
    if (success) {
      _delayOnFlag = false;
      _fanState = true;
      _fanSpeed = speed;
      _publishStatus();
      return true;
    }
  }
  else {
    if (_setOff()) {
      if (speed != _fanSpeed) {
        _fanSpeed = speed;
      }
      _delayOnFlag = false;
      _fanState = false;
      _publishStatus();
      return true;
    }
  }
  return false;
}

Fan_Pin::Fan_Pin(IotHandler* handler, const char* mqtt_object_id, int offPin, int lowPin, int medPin, int highPin, bool restingState, long relayActiveTime, const char* mqtt_component)
  : Fan(handler, mqtt_object_id, mqtt_component),
    _off_toggle(offPin, restingState, relayActiveTime),
    _low_toggle(lowPin, restingState, relayActiveTime),
    _med_toggle(medPin, restingState, relayActiveTime),
    _high_toggle(highPin, restingState, relayActiveTime)
{
  Serial.println("Start Fan_PinModule");
}

Fan_Pin::Fan_Pin(IotHandler* handler, const char* mqtt_object_id, int offPin, int lowPin, int medPin, int highPin, const char* mqtt_component)
  : Fan(handler, mqtt_object_id, mqtt_component),
    _off_toggle(offPin),
    _low_toggle(lowPin),
    _med_toggle(medPin),
    _high_toggle(highPin)
{
  Serial.println("Start Fan_PinModule");
}
Fan_Function::Fan_Function(IotHandler* handler, const char* mqtt_object_id,
                  void (*callback_off_on_function)(), void (*callback_off_off_function)(), 
                  void (*callback_low_on_function)(), void (*callback_low_off_function)(), 
                  void (*callback_med_on_function)(), void (*callback_med_off_function)(), 
                  void (*callback_high_on_function)(), void (*callback_high_off_function)(), 
                  bool restingState, long relayActiveTime, const char* mqtt_component)
  : Fan(handler, mqtt_object_id, mqtt_component),
    _off_toggle(callback_off_on_function, callback_off_off_function, restingState, relayActiveTime),
    _low_toggle(callback_low_on_function, callback_low_off_function, restingState, relayActiveTime),
    _med_toggle(callback_med_on_function, callback_med_off_function, restingState, relayActiveTime),
    _high_toggle(callback_high_on_function, callback_high_off_function, restingState, relayActiveTime)
{
  Serial.println("Start Fan_PinModule");
}

Fan_Function::Fan_Function(IotHandler* handler, const char* mqtt_object_id,
                  void (*callback_off_on_function)(), void (*callback_off_off_function)(), 
                  void (*callback_low_on_function)(), void (*callback_low_off_function)(), 
                  void (*callback_med_on_function)(), void (*callback_med_off_function)(), 
                  void (*callback_high_on_function)(), void (*callback_high_off_function)(), 
                  const char* mqtt_component)
  : Fan(handler, mqtt_object_id, mqtt_component),
    _off_toggle(callback_off_on_function, callback_off_off_function),
    _low_toggle(callback_low_on_function, callback_low_off_function),
    _med_toggle(callback_med_on_function, callback_med_off_function),
    _high_toggle(callback_high_on_function, callback_high_off_function)
{
  Serial.println("Start Fan_PinModule");
}
/*
Fan_Function::Fan_Function(IotHandler* handler, const char* mqtt_object_id, int offPin, int lowPin, int medPin, int highPin, const char* mqtt_component)
  : Fan(handler, mqtt_object_id, mqtt_component),
    _off_toggle(offPin),
    _low_toggle(lowPin),
    _med_toggle(medPin),
    _high_toggle(highPin)
{
  Serial.println("Start Fan_PinModule");
}
*/
