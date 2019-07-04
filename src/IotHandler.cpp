#include "IotHandler.h"

/*
   wifi_ssid - Wifi SSID
   wifi_password - wifi password
   mqtt_broker - IP of the mqtt broker sever
   int mqtt_port - port of mqtt server
   mqtt_node_id - the name of the ESP Node
   mqtt_username - the username to login to mqtt server
   mqtt_password - the password to login to mqtt server
   discover_prefix - the discovery prefix for mqtt server - https://www.home-assistant.io/docs/mqtt/discovery/
*/

void IotHandler::setParams(const char* wifi_ssid, const char* wifi_password, const char* mqtt_broker, int mqtt_port, const char* mqtt_node_id, const char* mqtt_username, const char* mqtt_password, const char* discover_prefix) {
  // WIFI Params
  _wifi_ssid = wifi_ssid;
  _wifi_password = wifi_password;

  _wifiState = false;

  // MQTT PARAMS
  _mqtt_broker = mqtt_broker;
  _mqtt_port = mqtt_port;
  _mqtt_node_id = mqtt_node_id;
  _mqtt_username = mqtt_username;
  _mqtt_password = mqtt_password;
  _discover_prefix = discover_prefix;

  //<discovery_prefix>/<component>/<node_id>/<object_id>/<topic>.
  //<discovery_prefix>/module/<node_id>/availible
  const char *availibility_suffix = MQTT_AVAILIBILITY_TOPIC_SUFFIX;
  strcpy(_availabilityTopic, _discover_prefix);
  strcat(_availabilityTopic, "/module/");
  strcat(_availabilityTopic, _mqtt_node_id);
  strcat(_availabilityTopic, "/");
  strcat(_availabilityTopic, availibility_suffix);

  // OTA PARAMS
//  const char *ota_prefix = OTA_HOSTNAME_PREFIX;
  
  snprintf(_otaName, 20, "%s_%X-%s", mqtt_node_id, ESP.getChipId(), OTA_HOSTNAME_PREFIX);
//  strcpy(_otaName, ota_prefix);
//  strcat(_otaName, mqtt_node_id);
//  strcat(_otaName, ESP.getChipId().c_str());
}

void IotHandler::setup() {
  _client.setClient(espClient);
  delay(10);
  _client.setCallback(actionCaller);

  setupWiFi();
  setupOTA();
  setupMQTT();

  yield();

  for (int i = 0; i < _index; i++) {
    Serial.printf("Run Module %d Setup\n", i);
    _module_array[i]->_setup();
    _client.loop();
    yield();
  }
  Serial.println("Ready");
}


bool IotHandler::addModule(Module* module) {
  if (_index + 1 <= _MODULE_ARRAY_SIZE) {
    _module_array[_index] = module;
    Serial.printf("\tModule %d, %s, Added.\n", _index, module->_mqtt_object_id);
    _index++;
    return true;
  }
  else {
    Serial.println("\t\t\tERROR - Module Not Added");
    return false;
  }
}

/*************************************************************************
 ********************** WiFi *********************************************
 *************************************************************************/

void IotHandler::setupWiFi() {

  Serial.println("\n\nBooting...");


  WiFi.disconnect();
  Serial.println("Starting WiFi...");
  Serial.print("Connecting to ");
  Serial.println(_wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_wifi_ssid, _wifi_password);
}

void IotHandler::reconnectWiFi() {
  static unsigned long setupMillis = 0;
  static unsigned long restartMillis = 0;
  const unsigned long setupInterval = WIFI_SETUP_INTERVAL;
  const unsigned long restartInterval = WIFI_RESTART_INTERVAL;

  if (WiFi.status() != WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (_wifiState == true) {
      Serial.println("Wifi was lost...");
      _wifiState = false;
      setupMillis = currentMillis;
      restartMillis = currentMillis;
    }
    if (currentMillis - setupMillis >= setupInterval) {
      Serial.println("Connection Failed! Reseting WiFi...");
      setupMillis = currentMillis;
      delay(100);
      setupWiFi();
    }
    else if (currentMillis - restartMillis >= restartInterval) {
      Serial.println("Connection Failed! Rebooting...");
      delay(500);
      ESP.restart();
    }
  }
  else if (_wifiState == false) {
    _wifiState = true;
    Serial.print("Connected!\nIP address: ");
    Serial.print(WiFi.localIP()); //Print the local IP
    Serial.print(" MAC address: ");
    Serial.println(WiFi.macAddress());
    //    setupMQTT(); //not sure if this is needed. Seems to fail after wifi goes down and is reconnected.
  }
}

/*************************************************************************
 ********************** MQTT *********************************************
 *************************************************************************/

// Setup the MQTT connection to begin

void IotHandler::setupMQTT() {
  _client.disconnect();
  _client.setServer(_mqtt_broker, _mqtt_port);
}

// Function that runs in loop() to connect/reconnect to the MQTT broker, and publish the current statuses on connect

void IotHandler::reconnectMQTT() {
  static unsigned long reconnectMillis = 0;
  const unsigned long reconnectInterval = MQTT_RECONNECT_INTERVAL;
  unsigned long currentMillis = millis();


  if (currentMillis - reconnectMillis >= reconnectInterval) {
    reconnectMillis = currentMillis;
    digitalWrite(_wifi_pin, false);
    Serial.print("Attempting MQTT connection...");


    if (_client.connect(_otaName, _mqtt_username, _mqtt_password, _availabilityTopic, 0, true, _lwtMessage)) {
      //    if (_client.connect(_mqtt_node_id, _mqtt_username, _mqtt_password, "TEST/AVAILIBILITY", 0, true, _lwtMessage)) {
      Serial.print("\n");
      publish_birth_message();
      _onConnect();
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(_client.state());
      Serial.print(" try again in ");
      Serial.print(reconnectInterval);
      Serial.println(" ms");
    }
  }
}

void IotHandler::_triggerAction(char* topic, byte* payload, unsigned int length) {
  bool action_flag = false;
  String topicToProcess = topic;
  payload[length] = '\0';
  String payloadToProcess = (char*)payload;

  Serial.printf("%d: Message Arrived: %s from %s\n", millis(), payloadToProcess.c_str(), topicToProcess.c_str());

  for (int i = 0; i < _index; i++) {
    if (_module_array[i]->_handleAction(topicToProcess, payloadToProcess)) {
      action_flag = true;
      _client.loop();
    }
  }

  if (!action_flag) {
    Serial.println("Unrecognized topic / payload... taking no action!");
  }
}

void IotHandler::_onConnect() {
  for (int i = 0; i < _index; i++) {
    _module_array[i]->_onConnect();
    _client.loop();
  }
}

// Function that publishes birthMessage
void IotHandler::publish_birth_message() {
  // Publish the birthMessage
  Serial.print("Publishing birth message \"");
  Serial.print(_birthMessage);
  Serial.print("\" to ");
  Serial.print(_availabilityTopic);
  Serial.println("...");
  _client.publish(_availabilityTopic, _birthMessage, true);
}

void IotHandler::printMQTTState(int state) {
  switch (state) {
    case -4:
      Serial.print("MQTT_CONNECTION_TIMEOUT");
      break;
    case -3:
      Serial.print("MQTT_CONNECTION_LOST");
      break;
    case -2:
      Serial.print("MQTT_CONNECT_FAILED");
      break;
    case -1:
      Serial.print("MQTT_DISCONNECTED");
      break;
    case 0:
      Serial.print("MQTT_CONNECTED");
      break;
    case 1:
      Serial.print("MQTT_CONNECT_BAD_PROTOCOL");
      break;
    case 2:
      Serial.print("MQTT_CONNECT_BAD_CLIENT_ID");
      break;
    case 3:
      Serial.print("MQTT_CONNECT_UNAVAILABLE");
      break;
    case 4:
      Serial.print("MQTT_CONNECT_BAD_CREDENTIALS");
      break;
    case 5:
      Serial.print("MQTT_CONNECT_UNAUTHORIZED");
      break;
    default:
      Serial.print("N/A");
      break;
  }
}

/*************************************************************************
 ********************** Loop Functions ***********************************
 *************************************************************************/

void IotHandler::loop() {
  ArduinoOTA.handle();
  _client.loop();

  for (int i = 0; i < _index; i++) {
    //    Serial.printf("Run Module %d\n", i);
    _module_array[i]->_loop();
    _client.loop();
  }

  static int mqttStatus = -99;
  if (mqttStatus != _client.state()) {
    Serial.print("\tMQTT Status Changed from ");
    printMQTTState(mqttStatus);
    Serial.print(" to ");
    mqttStatus = _client.state();
    printMQTTState(mqttStatus);
    Serial.println();
  }

  bool wifi = WiFi.status() == WL_CONNECTED;
  bool mqtt = _client.connected();

  reconnectWiFi();
  if (wifi & !mqtt) {
    // Connect/reconnect to the MQTT broker and listen for messages
    reconnectMQTT();
  }

  heartBeat(wifi, mqtt);
}

void IotHandler::heartBeat(bool wifi, bool mqtt) {
  static byte hbMode = 0;
  static unsigned long previousMillis = 0;
  static int ledState = LOW;
  const unsigned long interval = HEARTBEAT;
  unsigned long currentMillis = millis();

  //  Serial.println(hbMode);
  if (!wifi && hbMode != 1) {
    Serial.println("HeartBeatMode Changed: Waiting for Wifi...");
    digitalWrite(_led_pin, LOW); // switches the redLED on
    ledState = LOW;
    previousMillis = currentMillis - interval;
    hbMode = 1;
  }
  else if (!mqtt && wifi && hbMode != 2) {
    Serial.println("HeartBeatMode Changed: Waiting for MQTT...");
    digitalWrite(_led_pin, HIGH); // switches the LED off
    ledState = LOW;
    hbMode = 2;
    previousMillis = currentMillis - interval;
  }
  else if (wifi && mqtt && hbMode != 0) {
    Serial.println("HeartBeatMode Changed: Running!");
    digitalWrite(_wifi_pin, HIGH);
    ledState = LOW;
    hbMode = 0;
    previousMillis = currentMillis - interval;
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (hbMode == 0) {
      digitalWrite(_led_pin, ledState);
    }
    else {
      digitalWrite(_wifi_pin, ledState);
    }
    ledState = !ledState;
  }
}


/*************************************************************************
 ********************** OTA **********************************************
 *************************************************************************/

void IotHandler::setupOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(_otaName);

  // No authentication by default
  //#ifdef OTA_PASSWORD
  //  ArduinoOTA.setPassword(otaPassword);
  //#endif

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}


