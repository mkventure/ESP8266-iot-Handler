#include "IotHandler.h"
#include "Arduino.h"

#ifndef Module_h
#define Module_h

#define ACTIVE_HIGH_RELAY true
#define RELAY_ACTIVE_TIME 250

#define MQTT_ACTION_TOPIC_SUFFIX "SET"
#define MQTT_STATUS_TOPIC_SUFFIX "STATE"
#define MQTT_COMMAND_TOPIC_SUFFIX "COMMAND"


class Module
{
  public:
    Module(IotHandler*);
    Module(IotHandler*, const char*);
    void onConnect();
    bool triggerAction(String topic, String payload);
    void loop();

  protected:
    IotHandler* handler;
    void _setupHandler(IotHandler*);
//    void _setupPins();
    void _publishStatus();
    void _setupTopic(char*, const char*);
    void _setupTopic(char*, const char*, const char*);
    void _setupTopic(char*, const char*, const char*, const char*);

    char _mqtt_status_topic[40];
    char _mqtt_command_topic[40];
};

class ActionModule : public Module
{
  public:
    ActionModule(IotHandler*);
    ActionModule(IotHandler*, const char*);
    void onConnect();
    
  protected:
    char _mqtt_action_topic[40];
};


class ToggleRelayModule : public ActionModule
{
  public:
    ToggleRelayModule(IotHandler*, const char*, bool activeHighRelay = ACTIVE_HIGH_RELAY, long relayActiveTime = RELAY_ACTIVE_TIME);
    
  protected:
    void _toggleRelay(int pin);

    bool _activeHighRelay;
    long _relayActiveTime;
    
};


#endif
