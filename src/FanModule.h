#include "Arduino.h"

#ifndef FanModule_h
#define FanModule_h

#include "IotHandler.h"


#define ACTIVE_HIGH_RELAY true
#define ACTION_TIME 0
#define RELAY_ACTIVE_TIME 500
#define FAN_ON_DELAY 50

#define MQTT_FAN_ACTION_TOPIC_SUFFIX "/FAN/SET"
#define MQTT_FAN_STATUS_TOPIC_SUFFIX "/FAN/STATE"
#define MQTT_FANSPEED_ACTION_TOPIC_SUFFIX "/FAN/SPEED/SET"
#define MQTT_FANSPEED_STATUS_TOPIC_SUFFIX "/FAN/SPEED/STATE"
#define FAN_ON_PAYLOAD "FAN_ON"
#define FAN_OFF_PAYLOAD "FAN_OFF"
#define FAN_LOW_PAYLOAD "FAN_LOW"
#define FAN_MED_PAYLOAD "FAN_MED"
#define FAN_HIGH_PAYLOAD "FAN_HIGH"

enum FanSpeed {
  FAN_HIGH,
  FAN_MED,
  FAN_LOW
};

class FanModule
{
  public:
    FanModule(IotHandler*, int, int, int, int);

    void onConnect();
    bool triggerAction(String topic, String payload);

    void loop();

  private:
    void handle_FanOnDelay();
    void publish_fan_status();
    void publish_fanSpeed_status();
    void _toggleRelay(int pin);

    IotHandler* handler;

    int _fan_off_pin;
    int _fan_low_pin;
    int _fan_med_pin;
    int _fan_high_pin;

    enum FanSpeed _fanSpeed;
    enum Ternary _fan;

    const int _actionTime = ACTION_TIME;
    const int _relayActiveTime = RELAY_ACTIVE_TIME;

    char _mqtt_fan_action_topic[40];
    char _mqtt_fan_status_topic[40];
    char _mqtt_fanspeed_action_topic[40];
    char _mqtt_fanspeed_status_topic[40];

    bool _onFlag = false;
    const unsigned long fan_onDelay = FAN_ON_DELAY;
    unsigned long _fan_onTime;
};

#endif
