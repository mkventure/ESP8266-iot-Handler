#include "Arduino.h"
#ifndef LightModule_h
#define LightModule_h

#include "IotHandler.h"

#define ACTIVE_HIGH_RELAY true
#define ACTION_TIME 0
#define RELAY_ACTIVE_TIME 500

#define MQTT_LIGHT_ACTION_TOPIC_SUFFIX "/LIGHT/SET"
#define MQTT_LIGHT_STATUS_TOPIC_SUFFIX "/LIGHT/STATE"
#define MQTT_LIGHT_COMMAND_TOPIC_SUFFIX "/LIGHT/COMMAND"

#define LIGHT_INVERT_PAYLOAD "INVERT_LIGHT"
#define LIGHT_ON_PAYLOAD "LIGHT_ON"
#define LIGHT_OFF_PAYLOAD "LIGHT_OFF"



class LightModule
{
  public:
    LightModule(IotHandler*, int, int);
    LightModule(IotHandler*, int);

    void onConnect();
    bool triggerAction(String topic, String payload);

    void loop();

  private:
    void setupPins(int, int);
    void setupHandler(IotHandler*);
    void publish_light_status();
    void toggleRelay(int pin);

    IotHandler* handler;

    int _light_off_pin;
    int _light_on_pin;

    enum Ternary _light = STATE_UNKNOWN;

    const int _actionTime = ACTION_TIME;
    const int _relayActiveTime = RELAY_ACTIVE_TIME;

    char _mqtt_light_action_topic[40];
    char _mqtt_light_status_topic[40];
    char _mqtt_light_command_topic[40];
};


#endif
