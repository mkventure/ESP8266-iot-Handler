#include "Arduino.h"
#ifndef iotHandler_h
#define iotHandler_h

#define MQTT_CALLBACK_ACTION_SIGNATURE void (*callback)(char*, uint8_t*, unsigned int)
#define MQTT_CALLBACK_CONNECT_SIGNATURE void (*callbackConnect)()


#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>


class iotHandler
{
  public:
    iotHandler(int wifi_pin, int led_pin, const char*, const char*, const char*, const char*, const char*, const char*);
    void loop();

    void set_onAction_callback(MQTT_CALLBACK_ACTION_SIGNATURE);
    void set_onConnect_callback(MQTT_CALLBACK_CONNECT_SIGNATURE);

    bool debugPrint;
    bool debugLed;
    
    WiFiClient espClient;
    PubSubClient client;

  private:


    int _wifi_pin;
    int _led_pin;

    const char* _wifi_ssid;
    const char* _wifi_password;
    bool _wifiState = false;

    const char* _mqtt_broker;
    const char* _mqtt_clientId;
    const char* _mqtt_username;
    const char* _mqtt_password;
    int _mqtt_port;

    const char* _birthMessage;
    const char* _lwtMessage;
    char _availabilityTopic[40];

    char _otaName[20]; 

    MQTT_CALLBACK_ACTION_SIGNATURE;
    MQTT_CALLBACK_CONNECT_SIGNATURE; //variable of the callback function
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

#endif
