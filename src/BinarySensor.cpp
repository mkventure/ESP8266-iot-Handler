#include "BinarySensor.h"

BinarySensor::BinarySensor(IotHandler* handler, int sensor_pin) {
  _setupPins(sensor_pin);
  _setupHandler(handler);
  _publish_sensor_status();
}

void BinarySensor::_setupPins(int sensor_pin) {
  _sensor_pin = sensor_pin;

  pinMode(_sensor_pin, INPUT_PULLUP);

  //Set sensor to known off state
  _sensor_state = digitalRead(_sensor_pin);
  _sensor_change = true;
}


void BinarySensor::_setupHandler(IotHandler* handler) {
  this->handler = handler;

  const char *binarysensor_status_topic_suffix = MQTT_BINARYSENSOR_STATUS_TOPIC_SUFFIX;
  strcpy(_mqtt_binarysensor_status_topic, handler->mqtt_clientId);
  strcat(_mqtt_binarysensor_status_topic, binarysensor_status_topic_suffix);

  const char *binarysensor_command_topic_suffix = MQTT_BINARYSENSOR_COMMAND_TOPIC_SUFFIX;
  strcpy(_mqtt_binarysensor_command_topic, handler->mqtt_clientId);
  strcat(_mqtt_binarysensor_command_topic, binarysensor_command_topic_suffix);
}

bool BinarySensor::getState() {
  return _sensor_state;
}

bool BinarySensor::getStateChange() {
  bool retVal = _sensor_change;
  _sensor_change = false;
  return retVal;
}

bool BinarySensor::_sensor_read() {
  static unsigned long _lastSwitchTime = 0;
  int currentStatusValue = digitalRead(_sensor_pin);
  if (currentStatusValue != _sensor_state) {
    unsigned int currentTime = millis();
    if (currentTime - _lastSwitchTime >= _debounceTime) {
      _sensor_state = currentStatusValue;
      _sensor_change = true;
      _lastSwitchTime = currentTime;
      return true;
    }
  }
  return false;
}

void BinarySensor::_publish_sensor_status() {
  Serial.print("Publishing Sensor State: ");
  if (_sensor_state == true) {
    handler->client.publish(_mqtt_binarysensor_status_topic, _mqtt_binarysensor_on_payload, true);
    Serial.println(_mqtt_binarysensor_on_payload);
  }
  else if (_sensor_state == false) {
    handler->client.publish(_mqtt_binarysensor_status_topic, _mqtt_binarysensor_off_payload, true);
    Serial.println(_mqtt_binarysensor_off_payload);
  }
}

bool BinarySensor::triggerAction(String topic, String payload) {
  static unsigned long _lastActionTime = 0;
  unsigned int currentTime = millis();

  if (topic == _mqtt_binarysensor_command_topic && payload == _mqtt_binarysensor_read_payload && currentTime - _lastActionTime >= _actionTime) {
    _lastActionTime = currentTime;
    _sensor_read();
    _publish_sensor_status();
  }
  else {
    return false;
  }
  return true;
}

void BinarySensor::onConnect() {
  Serial.print("Subscribing to ");
  Serial.print(_mqtt_binarysensor_command_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_binarysensor_command_topic);

  // Publish the current status on connect/reconnect
  _sensor_read();
  _publish_sensor_status();
}

void BinarySensor::loop() {
  if (_sensor_read()) {
    _publish_sensor_status();
  }
}


