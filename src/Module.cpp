#include "IotHandler.h"


Module::Module(IotHandler* handler, const char* mqtt_object_id, const char* mqtt_component)
  : _handler(handler), _mqtt_object_id(mqtt_object_id), _mqtt_component(mqtt_component)
{
  Serial.println("Start Module");
  _handler->addModule(this);
}

void Module::_setup() {
  Serial.println("Setup Module");
  _setupTopic(_mqtt_config_topic, MQTT_CONFIG_TOPIC_SUFFIX);
  _setupTopic(_mqtt_status_topic, MQTT_STATUS_TOPIC_SUFFIX);
  _addConfigElement("~", _getBaseTopic());
  _addConfigElement("name", _mqtt_object_id);
  char uid[21];
  snprintf(uid, 20, "%X-%s", ESP.getChipId(), _mqtt_object_id);
  _addConfigElement("unique_id", uid);
  char device[111];
  snprintf(device, 110, "{\"model\": \"IotHandler\", \"identifiers\": \"%X\", \"connections\": [[\"mac\", \"%s\"]]}", ESP.getChipId(), WiFi.macAddress().c_str());
  _addConfigElement("\"device\"", device, false);
  _addConfigElement("stat_t", "~/state");
  _addConfigElement("avty_t", _handler->_getAvailibilityTopic());
}

//<discovery_prefix>/<component>/<node_id>/<object_id>/<topic>.
void Module::_setupTopic(char* topic, const char* suffix) {
  strcpy(topic, _handler->_discover_prefix);
  strcat(topic, "/");
  strcat(topic, _mqtt_component);
  strcat(topic, "/");
  strcat(topic, _handler->_mqtt_node_id);
  strcat(topic, "/");
  strcat(topic, _mqtt_object_id);
  strcat(topic, "/");
  strcat(topic, suffix);
}

void Module::_setupTopic(char* topic, const char* suffix, const char* suffix2) {
  _setupTopic(topic, suffix);
  strcat(topic, "/");
  strcat(topic, suffix2);
}
String Module::_getBaseTopic() {
  String s = _handler->_discover_prefix;
  s.concat("/");
  s.concat(_mqtt_component);
  s.concat("/");
  s.concat(_handler->_mqtt_node_id);
  s.concat("/");
  s.concat(_mqtt_object_id);
  return s;
}

void Module::_addConfigElement(String key, String value, bool addQuotes) {
  switch (_completeConfig) {
    case newState:
      _completeConfig = inproces;
      break;
    case inproces:
      _mqtt_config_payload.concat(", ");
      break;
    case completed:
      Serial.println("Unable to add config element to completed config");
      return;
  }
  if (addQuotes) {
    _mqtt_config_payload.concat("\"");
  }
  _mqtt_config_payload.concat(key);
  if (addQuotes) {
    _mqtt_config_payload.concat("\": \"");
  } else {
    _mqtt_config_payload.concat(": ");
  }
  _mqtt_config_payload.concat(value);
  if (addQuotes) {
    _mqtt_config_payload.concat("\"");
  }
}

void Module::_onConnect() {
  bool success = getClient().publish(_mqtt_config_topic, _getConfigPayload().c_str(), true);
  Serial.printf("Submitting Config: %x \n\t%s to \n\t%s\n", success, _getConfigPayload().c_str(), _mqtt_config_topic);
//  Serial.println(getClient().publish(_mqtt_config_topic, "", true));
//  delay(50);
  _publishStatus();
}

String Module::_getConfigPayload() {
  if (_completeConfig != completed) {
    _mqtt_config_payload.concat("}");
    _completeConfig = completed;
  }
  return _mqtt_config_payload;
}

ActionModule::ActionModule(IotHandler* handler, const char* modName, const char* mqtt_component)
  : Module(handler, modName, mqtt_component)
{
  Serial.println("Start ActionModule");
}

void ActionModule::_setup() {
  Module::_setup();
  Serial.println("Setup ActionModule");
  _setupTopic(_mqtt_command_topic, MQTT_COMMAND_TOPIC_SUFFIX);
  _addConfigElement("cmd_t", "~/set");
}

void ActionModule::_onConnect() {
  Module::_onConnect();
  Serial.printf("Subscribing to %s\n", _mqtt_command_topic);
  getClient().subscribe(_mqtt_command_topic);
  yield();
}


