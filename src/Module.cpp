#include "Module.h"

Module::Module(IotHandler* handler) {
  _setupHandler(handler);
  _setupTopic(_mqtt_status_topic, MQTT_STATUS_TOPIC_SUFFIX);
  _setupTopic(_mqtt_command_topic, MQTT_COMMAND_TOPIC_SUFFIX);
}

Module::Module(IotHandler* handler, const char* modName) {
  _setupHandler(handler);
  _setupTopic(_mqtt_status_topic, MQTT_STATUS_TOPIC_SUFFIX, modName);
  _setupTopic(_mqtt_command_topic, MQTT_COMMAND_TOPIC_SUFFIX, modName);
}

void Module::onConnect() {
  Serial.print("Subscribing to ");
  Serial.print(_mqtt_command_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_command_topic);
}

//input topic / commands that are received.
//return if action has been triggered
//function checks topic / payload against relivant ones and triggers actions as necessary.
bool Module::triggerAction(String topic, String payload) {
  return false;
}

void Module::loop() {

}

void Module::_setupHandler(IotHandler* handler) {
  this->handler = handler;
}

void Module::_publishStatus() {
  Serial.println("Publish not defined in Module");
}

void Module::_setupTopic(char* topic, const char* suffix) {
  strcpy(topic, handler->mqtt_clientId);
  strcat(topic, "/");
  //  strcat(topic, suffix);
}

void Module::_setupTopic(char* topic, const char* suffix, const char* modName) {
  strcpy(topic, handler->mqtt_clientId);
  strcat(topic, "/");
  strcat(topic, modName);
  strcat(topic, "/");
  strcat(topic, suffix);
}

void Module::_setupTopic(char* topic, const char* suffix, const char* prefix, const char* modName) {
  strcpy(topic, handler->mqtt_clientId);
  strcat(topic, "/");
  strcat(topic, modName);
  strcat(topic, "/");
  strcat(topic, prefix);
  strcat(topic, "/");
  strcat(topic, suffix);
}


ActionModule::ActionModule(IotHandler* handler)
  : Module(handler)
{
  _setupTopic(_mqtt_action_topic, MQTT_ACTION_TOPIC_SUFFIX);
}

ActionModule::ActionModule(IotHandler* handler, const char* modName)
  : Module(handler, modName)
{
  _setupTopic(_mqtt_action_topic, MQTT_ACTION_TOPIC_SUFFIX, modName);
}

void ActionModule::onConnect() {
  Module::onConnect();
  Serial.print(" & ");
  Serial.print(_mqtt_action_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_action_topic);
}

ToggleRelayModule::ToggleRelayModule(IotHandler* handler, const char* modName, bool activeHighRelay , long relayActiveTime)
  : ActionModule(handler, modName)
{
  _activeHighRelay = activeHighRelay;
  _relayActiveTime = relayActiveTime;
}

void ToggleRelayModule::_toggleRelay(int pin) {
  if (_activeHighRelay) {
    digitalWrite(pin, HIGH);
    Serial.print("Toggled HIGH to Pin: ");
    Serial.println(pin);
    delay(_relayActiveTime);
    digitalWrite(pin, LOW);
  }
  else {
    digitalWrite(pin, LOW);
    Serial.print("Toggled LOW to Pin: ");
    Serial.println(pin);
    delay(_relayActiveTime);
    digitalWrite(pin, HIGH);
  }
}


