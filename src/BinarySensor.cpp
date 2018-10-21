#include "BinarySensor.h"

BinarySensor::BinarySensor(IotHandler* handler, const char* modName)
  : Module(handler, modName)
{
  _changeTime = millis();
  _sensorState = false;
}

void BinarySensor::onConnect() {
  Module::onConnect();
  _publishStatus();
}

bool BinarySensor::triggerAction(String topic, String payload) {
  if (topic == Module::_mqtt_command_topic && payload == _mqtt_binarysensor_read_payload) {
    _publishStatus();
  }
  else {
    return false;
  }
  return true;
}

bool BinarySensor::setState(bool state) {
  if (_sensorState != state) {
    _changeTime = millis();
    _sensorState = state;
    return true;
  }
  else {
    return false;
  }
}

bool BinarySensor::getState() {
  return _sensorState;
}

unsigned long BinarySensor::getChangeTime() {
  return _changeTime;
}

void BinarySensor::_publishStatus() {
  Serial.print("Publishing Sensor State: ");
  if (_sensorState == true) {
    Module::handler->client.publish(Module::_mqtt_status_topic, _mqtt_binarysensor_on_payload, true);
    Serial.println(_mqtt_binarysensor_on_payload);
  }
  else if (_sensorState == false) {
    Module::handler->client.publish(Module::_mqtt_status_topic, _mqtt_binarysensor_off_payload, true);
    Serial.println(_mqtt_binarysensor_off_payload);
  }
}

BinarySensor_Pin::BinarySensor_Pin(IotHandler* handler, int pin, const char* modName, int debounceTime)
  : BinarySensor(handler, modName)
{
  _setupPins(pin);
  _debounceTime = debounceTime;
}

//has the state changes since function last called
bool BinarySensor_Pin::getStateChange() {
  if (_sensorChange) {
    _sensorChange = false;
    return true;
  }
  return false;
}

//redefine loop to read sensor on each loop.
void BinarySensor_Pin::loop() {
  if (_sensorRead()) {
    _publishStatus();
  }
}

void BinarySensor_Pin::_setupPins(int sensorPin) {
  _sensorPin = sensorPin;
  pinMode(_sensorPin, INPUT_PULLUP);
  //Set sensor to known off state
  _sensorState = digitalRead(_sensorPin);
  _sensorChange = true;
}

bool BinarySensor_Pin::_sensorRead() {
  static unsigned long __lastDebounceTime = 0;
  static bool __lastState = _sensorState; //TODO - does this work? or does it need to be a constant?
  bool change = false;
  
  bool reading = digitalRead(_sensorPin);
  if (reading != __lastState) {
    __lastDebounceTime = millis();
  } 
  
  if ((millis() - __lastDebounceTime) > _debounceTime) {
    change = BinarySensor::setState(reading);
  }
  
  __lastState = reading;
  return change;
}

