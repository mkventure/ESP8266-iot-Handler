#include "Fan.h"

Fan::Fan(IotHandler* handler, int offPin, int lowPin, int medPin, int highPin, bool activeHighRelay, long relayActiveTime, const char* modName, const char* speedName)
  : ToggleRelayModule(handler, modName, activeHighRelay, relayActiveTime)
{
  _setupPins(offPin, lowPin, medPin, highPin);
  _setupTopic(_mqtt_fanspeed_action_topic, MQTT_ACTION_TOPIC_SUFFIX, speedName, modName);
  _setupTopic(_mqtt_fanspeed_status_topic, MQTT_STATUS_TOPIC_SUFFIX, speedName, modName);
}

void Fan::onConnect() {                                 //redefine onConnect
  ToggleRelayModule::onConnect();
  Serial.print(" & ");
  Serial.print(_mqtt_fanspeed_action_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_fanspeed_action_topic);
  _publishStatus();
}

bool Fan::triggerAction(String topic, String payload) { //redefine triggerAction
  if (topic == _mqtt_action_topic && payload == _mqtt_fan_on_payload) {
    if (_fan_onDelay_bool) {
      _onFlag = true;
      _fan_onTime = millis() + _fan_onDelay;
    }
    else {
      _fanState = true;
      if (_fanSpeed == FAN_HIGH) {
        _toggleRelay(_fan_high_pin);
      }
      else if (_fanSpeed == FAN_MED) {
        _toggleRelay(_fan_med_pin);
      }
      else if (_fanSpeed == FAN_LOW) {
        _toggleRelay(_fan_low_pin);
      }
      _publishStatus();
    }
  }
  else if (topic == _mqtt_action_topic && payload == _mqtt_fan_off_payload) {
    _toggleRelay(_fan_off_pin);
    _fanState = false;
    _publishStatus();
    _onFlag = false;
  }
  else if (topic == _mqtt_fanspeed_action_topic && payload == _mqtt_fanSpeed_high_payload) {
    _fanSpeed = FAN_HIGH;
    if (_fanState == true) {
      _toggleRelay(_fan_high_pin);
    }
    _publishStatus();
    _onFlag = false;
  }
  else if (topic == _mqtt_fanspeed_action_topic && payload == _mqtt_fanSpeed_med_payload) {
    _fanSpeed = FAN_MED;
    if (_fanState == true) {
      _toggleRelay(_fan_med_pin);
    }
    _publishStatus();
    _onFlag = false;
  }
  else if (topic == _mqtt_fanspeed_action_topic && payload == _mqtt_fanSpeed_low_payload) {
    _fanSpeed = FAN_LOW;
    if (_fanState == true) {
      _toggleRelay(_fan_low_pin);
    }
    _publishStatus();
    _onFlag = false;
  }
  else {
    return false;
  }
  return true;
}
void Fan::loop() {                                       //redefine loop
  _handle_FanOnDelay();
}

void Fan::_setupPins(int offPin, int lowPin, int medPin, int highPin) {
  _fan_off_pin = offPin;
  _fan_low_pin = lowPin;
  _fan_med_pin = medPin;
  _fan_high_pin = highPin;
  pinMode(_fan_off_pin, OUTPUT);
  pinMode(_fan_low_pin, OUTPUT);
  pinMode(_fan_med_pin, OUTPUT);
  pinMode(_fan_high_pin, OUTPUT);
  if (_activeHighRelay) {
    // Set output pins LOW with an active-high relay
    digitalWrite(_fan_off_pin, LOW);
    digitalWrite(_fan_low_pin, LOW);
    digitalWrite(_fan_med_pin, LOW);
    digitalWrite(_fan_high_pin, LOW);
  }
  else {
    // Set output pins HIGH with an active-low relay
    digitalWrite(_fan_off_pin, HIGH);
    digitalWrite(_fan_low_pin, HIGH);
    digitalWrite(_fan_med_pin, HIGH);
    digitalWrite(_fan_high_pin, HIGH);
  }

  //Set fan to known off state
  ToggleRelayModule::_toggleRelay(_fan_off_pin);
  _fanState = false;
  _fanSpeed = FAN_MED;
}

//The fan speed and state messages are received out of order. Only process the fan on commands if speed is not already affected.
void Fan::_handle_FanOnDelay() {
    if (_fan_onDelay_bool && _onFlag && millis() >= _fan_onTime) {
    _onFlag = false;
    _fanState = true;
    if (_fanSpeed == FAN_HIGH) {
      ToggleRelayModule::_toggleRelay(_fan_high_pin);
    }
    else if (_fanSpeed == FAN_MED) {
      ToggleRelayModule::_toggleRelay(_fan_med_pin);
    }
    else if (_fanSpeed == FAN_LOW) {
      ToggleRelayModule::_toggleRelay(_fan_low_pin);
    }
    Serial.print("Delayed On... ");
    _publishStatus();
  }
}

void Fan::_publishStatus() {
  _publishFanStatus();
  _publishSpeedStatus();
}

void Fan::_publishFanStatus() {
  Serial.print("Publishing Fan Speed: ");
  if (_fanState) {
    handler->client.publish(_mqtt_status_topic, FAN_ON_PAYLOAD, true);
    Serial.println(FAN_ON_PAYLOAD);
  }
  else {
    handler->client.publish(_mqtt_status_topic, FAN_OFF_PAYLOAD, true);
    Serial.println(FAN_OFF_PAYLOAD);
  }
}

void Fan::_publishSpeedStatus() {
  Serial.print("Publishing Fan Speed: ");
  if (_fanSpeed == FAN_LOW) {
    handler->client.publish(_mqtt_fanspeed_status_topic, FAN_LOW_PAYLOAD, true);
    Serial.println(FAN_LOW_PAYLOAD);
  }
  else if (_fanSpeed == FAN_MED) {
    handler->client.publish(_mqtt_fanspeed_status_topic, FAN_MED_PAYLOAD, true);
    Serial.println(FAN_MED_PAYLOAD);
  }
  else if (_fanSpeed == FAN_HIGH) {
    handler->client.publish(_mqtt_fanspeed_status_topic, FAN_HIGH_PAYLOAD, true);
    Serial.println(FAN_HIGH_PAYLOAD);
  }
}

