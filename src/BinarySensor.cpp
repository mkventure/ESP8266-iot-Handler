#include "BinarySensor.h"

BinarySensor::BinarySensor(IotHandler* handler, const char* mqtt_object_id, const char* mqtt_device)
  : Module(handler, mqtt_object_id, MQTT_COMPONENT_BINARYSENSOR), _debounceTime(0)
{
  Serial.println("Start BinarySensor");
  if (strncmp(mqtt_device, "None", 4) != 0) {
    Serial.printf("device class set to %s\n", mqtt_device);
    _addConfigElement("dev_cla", mqtt_device);
  }
}

BinarySensor::BinarySensor(IotHandler* handler, const char* mqtt_object_id, int debounceTime, const char* mqtt_device)
  : Module(handler, mqtt_object_id, MQTT_COMPONENT_BINARYSENSOR), _debounceTime(debounceTime)
{
  Serial.println("Start BinarySensor");
  if (strncmp(mqtt_device, "None", 4) != 0) {
    Serial.printf("device class set to %s\n", mqtt_device);
    _addConfigElement("dev_cla", mqtt_device);
  }
}

void BinarySensor::_publishStatus() {
  if (getState()) {
    Serial.printf("Publishing BinarySensor State: %s to %s\n", _mqtt_state_on_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_on_payload, true);
  }
  else {
    Serial.printf("Publishing BinarySensor State: %s to %s\n", _mqtt_state_off_payload, _mqtt_status_topic);
    getClient().publish(_mqtt_status_topic, _mqtt_state_off_payload, true);
  }
}

void BinarySensor::_loop() {
  readState();
}

bool BinarySensor::readState() {
  static unsigned long __lastDebounceTime = millis();
  static bool __lastState = _sensorState;

  bool change = false;

  bool reading = readHWState();
  if (reading != __lastState) {
    Serial.printf("\tSensor Bouncing: %x\n", reading);
    __lastDebounceTime = millis();
    __lastState = reading;
  }

  if (reading != _sensorState && (millis() - __lastDebounceTime) > _debounceTime) {
    Serial.printf("\tSensor Changeed: %x\n", reading);
    _sensorState = reading;
    _publishStatus();
    return true;
  }
  return false;
}


BinarySensor_Pin::BinarySensor_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, const char* mqtt_device)
  : BinarySensor(handler, mqtt_object_id, mqtt_device), _sensorPin(pin)
{
  Serial.println("Start BinarySensor_Pin");
  pinMode(_sensorPin, INPUT_PULLUP);
  _sensorState = digitalRead(_sensorPin);   //Set sensor to initial state
}

BinarySensor_Pin::BinarySensor_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, int debounceTime, const char* mqtt_device)
  : BinarySensor(handler, mqtt_object_id, debounceTime, mqtt_device), _sensorPin(pin)
{
  Serial.println("Start BinarySensor_Pin");
  pinMode(_sensorPin, INPUT_PULLUP);
  _sensorState = digitalRead(_sensorPin);   //Set sensor to initial state
}

bool BinarySensor_Pin::readHWState() {
  return digitalRead(_sensorPin);
}
