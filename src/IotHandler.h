#ifndef IotHandler_h
#define IotHandler_h

#include "Arduino.h"
#define _MODULE_ARRAY_SIZE 10
#define _MQTT_TOPIC_SIZE 80

#define WIFI_SETUP_INTERVAL 30000
#define WIFI_RESTART_INTERVAL 6000000
#define MQTT_RECONNECT_INTERVAL 10000
#define HEARTBEAT 250

#define MQTT_AVAILIBILITY_TOPIC_SUFFIX "available"
#define AVAILABLE_PAYLOAD "online"
#define UNAVAILABLE_PAYLOAD "offline"
#define OTA_HOSTNAME_PREFIX "OTA"

#define MQTT_CONFIG_TOPIC_SUFFIX "config"
#define MQTT_STATUS_TOPIC_SUFFIX "state"
#define MQTT_COMMAND_TOPIC_SUFFIX "set"

#define STATE_ON_PAYLOAD "ON"
#define STATE_OFF_PAYLOAD "OFF"

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

enum payloadStatus {
  newState,
  inproces,
  completed
};

class Module;

class IotHandler
{
  public:

    static IotHandler* getInstance() {
      static IotHandler _instance;
      Serial.begin(115200); //start serial right away for debug
//      Serial.println("IOT Instance Created");
      return &_instance;
    }
    void setParams(const char*, const char*, const char*, int, const char*, const char*, const char*, const char*);
    void setPins(bool, int, int);
    void loop();
    void setup();

  protected:
    PubSubClient _client;

    int _wifi_pin;
    int _led_pin;
    //TODO - implement toggle for LED use and serial debugs. Implement default for set pins
    //    bool debugPrint;
    //    bool debugLed;

    const char* _mqtt_node_id;
    const char* _discover_prefix;

    friend class Module;
    bool addModule(Module*);

    String _getAvailibilityTopic() {return String(_availabilityTopic);}

  private:
    IotHandler() {}; // Disallow instantiation outside of the class.
    WiFiClient espClient;

    void _onConnect();
    void _triggerAction(char*, uint8_t*, unsigned int);
    static void actionCaller(char* topic, byte* payload, unsigned int length) {
//      Serial.println("Action Called");
      IotHandler::getInstance()->_triggerAction(topic, payload, length);
    }

    int _index = 0;
    Module * _module_array [_MODULE_ARRAY_SIZE];

    const char* _wifi_ssid;
    const char* _wifi_password;
    bool _wifiState = false;

    const char* _mqtt_broker;
    const char* _mqtt_username;
    const char* _mqtt_password;
    int _mqtt_port;

    const char* _birthMessage = AVAILABLE_PAYLOAD;
    const char* _lwtMessage = UNAVAILABLE_PAYLOAD;
    char _availabilityTopic[_MQTT_TOPIC_SIZE];

    char _otaName[21];

    //    MQTT_CALLBACK_ACTION_SIGNATURE;  //variable of the action callback function
    //    MQTT_CALLBACK_CONNECT_SIGNATURE; //variable of the connect callback function
    static void onAction(char* topic, byte* payload, unsigned int length);

    void setupWiFi();
    void reconnectWiFi();
    void setupMQTT();
    void reconnectMQTT();
    void publish_birth_message();
    void printMQTTState(int state);
    void heartBeat(bool wifi, bool mqtt);
    void setupOTA();
};


//<discovery_prefix>/<component>/<node_id>/<object_id>/<topic>.

class Module
{
  protected:
    Module(IotHandler*, const char*, const char*);
    friend class IotHandler;

    IotHandler* _handler;
    const char* _mqtt_object_id;
    const char* _mqtt_component;

    virtual void _loop() {};
    virtual void _setup();  //setups the reference handler variables must occur after handler initalization completes, which will call this. 
    virtual void _onConnect();
    virtual void _publishStatus() = 0;

    virtual bool _handleAction(String topic, String payload) {
      return false;
    }
    PubSubClient getClient() {
      return _handler->_client;
    }

    void _setupTopic(char*, const char*);
    void _setupTopic(char*, const char*, const char*);
    String _getBaseTopic();
    char _mqtt_config_topic[_MQTT_TOPIC_SIZE];
    char _mqtt_status_topic[_MQTT_TOPIC_SIZE];

    void _addConfigElement(String, String, bool addQuotes = true);
    String _getConfigPayload();

  private:
    payloadStatus _completeConfig = newState;
    String _mqtt_config_payload = "{";
};


class ActionModule : public Module
{
  protected:
    ActionModule(IotHandler*, const char*, const char*);
    virtual void _setup();
    virtual void _onConnect();
    char _mqtt_command_topic[_MQTT_TOPIC_SIZE];
};


#endif

