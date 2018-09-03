#include "IotHandler.h"
#include "Arduino.h"

#define WIFI_SETUP_INTERVAL 30000
#define WIFI_RESTART_INTERVAL 6000000
#define MQTT_RECONNECT_INTERVAL 10000
#define HEARTBEAT 250

#define AVAILABLE_PAYLOAD "ONLINE"
#define UNAVAILABLE_PAYLOAD "OFFLINE"

#define MQTT_AVAILIBILITY_TOPIC_SUFFIX "/AVAILIBILITY"
#define OTA_HOSTNAME_PREFIX "OTA_"

IotHandler::IotHandler() {
}

IotHandler::IotHandler(int wifi_pin, int led_pin, const char* wifi_ssid, const char* wifi_password, const char* mqtt_broker, const char* mqtt_clientId, const char* mqtt_username, const char* mqtt_password)
{
  setPins(wifi_pin, led_pin);
  setParams(wifi_ssid, wifi_password, mqtt_broker, mqtt_clientId, mqtt_username, mqtt_password);
  setup();
}


void IotHandler::setPins(int wifi_pin, int led_pin) {
  this->wifi_pin = wifi_pin;
  this->led_pin = led_pin;

  pinMode(led_pin, OUTPUT);
  pinMode(wifi_pin, OUTPUT);
}

void IotHandler::setParams(const char* wifi_ssid, const char* wifi_password, const char* mqtt_broker, const char* mqtt_clientId, const char* mqtt_username, const char* mqtt_password) {
  // WIFI Params
  _wifi_ssid = wifi_ssid;
  _wifi_password = wifi_password;

  _wifiState = false;

  // MQTT PARAMS
  _mqtt_broker = mqtt_broker;
  this->mqtt_clientId = mqtt_clientId;
  _mqtt_username = mqtt_username;
  _mqtt_password = mqtt_password;
  _mqtt_port = 1883;

  _birthMessage = AVAILABLE_PAYLOAD;
  _lwtMessage = UNAVAILABLE_PAYLOAD;

  const char *availibility_suffix = MQTT_AVAILIBILITY_TOPIC_SUFFIX;
  strcpy(_availabilityTopic, mqtt_clientId);
  strcat(_availabilityTopic, availibility_suffix);

  // OTA PARAMS
  const char *ota_prefix = OTA_HOSTNAME_PREFIX;
  strcpy(_otaName, ota_prefix);
  strcat(_otaName, mqtt_clientId);

}
void IotHandler::setup() {
  client.setClient(espClient);
  delay(10);
  Serial.begin(115200);

  setupWiFi();
  setupOTA();
  setupMQTT();

  Serial.println("Ready");
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
  client.disconnect();
  client.setServer(_mqtt_broker, _mqtt_port);
  //  client.setCallback(callbackPrinter);
}

// Function that runs in loop() to connect/reconnect to the MQTT broker, and publish the current statuses on connect

void IotHandler::reconnectMQTT() {
  static unsigned long reconnectMillis = 0;
  const unsigned long reconnectInterval = MQTT_RECONNECT_INTERVAL;
  unsigned long currentMillis = millis();

  if (currentMillis - reconnectMillis >= reconnectInterval) {
    reconnectMillis = currentMillis;
    digitalWrite(wifi_pin, false);
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_clientId, _mqtt_username, _mqtt_password, _availabilityTopic, 0, true, _lwtMessage)) {
      Serial.print("\n");
      publish_birth_message();
      callbackConnect();
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.print(" try again in ");
      Serial.print(reconnectInterval);
      Serial.println(" ms");
    }
  }
}

void IotHandler::set_onAction_callback(MQTT_CALLBACK_ACTION_SIGNATURE) {
  client.setCallback(callback);
}

void IotHandler::set_onConnect_callback(MQTT_CALLBACK_CONNECT_SIGNATURE) {
  this->callbackConnect = callbackConnect;
}

// Callback when MQTT message is received; calls triggerAction(), passing topic and payload as parameters

//static void onAction(char* topic, byte* payload, unsigned int length) { //MQTT_CALLBACK_ACTION_SIGNATURE) { //char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//
//  Serial.println();
//
//  String topicToProcess = topic;
//  payload[length] = '\0';
//  String payloadToProcess = (char*)payload;
//  //  triggerAction(topicToProcess, payloadToProcess); //TODO
//}

// Function that publishes birthMessage

void IotHandler::publish_birth_message() {
  // Publish the birthMessage
  Serial.print("Publishing birth message \"");
  Serial.print(_birthMessage);
  Serial.print("\" to ");
  Serial.print(_availabilityTopic);
  Serial.println("...");
  client.publish(_availabilityTopic, _birthMessage, true);
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
  client.loop();

  static int mqttStatus = -99;
  if (mqttStatus != client.state()) {
    Serial.print("\tMQTT Status Changed from ");
    printMQTTState(mqttStatus);
    Serial.print(" to ");
    mqttStatus = client.state();
    printMQTTState(mqttStatus);
    Serial.println();
  }

  bool wifi = WiFi.status() == WL_CONNECTED;
  bool mqtt = client.connected();

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
    digitalWrite(led_pin, LOW); // switches the redLED on
    ledState = LOW;
    previousMillis = currentMillis - interval;
    hbMode = 1;
  }
  else if (!mqtt && wifi && hbMode != 2) {
    Serial.println("HeartBeatMode Changed: Waiting for MQTT...");
    digitalWrite(led_pin, HIGH); // switches the LED off
    ledState = LOW;
    hbMode = 2;
    previousMillis = currentMillis - interval;
  }
  else if (wifi && mqtt && hbMode != 0) {
    Serial.println("HeartBeatMode Changed: Running!");
    digitalWrite(wifi_pin, HIGH);
    ledState = LOW;
    hbMode = 0;
    previousMillis = currentMillis - interval;
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (hbMode == 0) {
      digitalWrite(led_pin, ledState);
    }
    else {
      digitalWrite(wifi_pin, ledState);
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

