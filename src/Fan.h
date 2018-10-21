#include "Arduino.h"
#ifndef FanModule_h
#define FanModule_h

#include "Module.h"

#define FAN_ON_DELAY 50

#define MQTT_FAN_NAME "FAN"
#define MQTT_FAN_SPEED_NAME "SPEED"

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

class Fan : public ToggleRelayModule
{
  public:
    Fan(IotHandler*, int, int, int, int, bool activeHighRelay = ACTIVE_HIGH_RELAY, long relayActiveTime = RELAY_ACTIVE_TIME, const char* modName = MQTT_FAN_NAME, const char* speedName = MQTT_FAN_SPEED_NAME);

    void onConnect();                                 //redefine onConnect
    bool triggerAction(String topic, String payload); //redefine triggerAction 
    void loop();                                      //redefine loop
    
  protected:
    void _setupPins(int, int, int, int);  
    void _handle_FanOnDelay();
    void _publishStatus();
    void _publishFanStatus();
    void _publishSpeedStatus();

    int _fan_off_pin;
    int _fan_low_pin;
    int _fan_med_pin;
    int _fan_high_pin;

    bool _fanState;
    enum FanSpeed _fanSpeed;

    bool _fan_onDelay_bool = true;
    const unsigned long _fan_onDelay = FAN_ON_DELAY;
    bool _onFlag = false;
    unsigned long _fan_onTime;
    
    char _mqtt_fanspeed_action_topic[40];
    char _mqtt_fanspeed_status_topic[40];
    
    const char* _mqtt_fan_on_payload = FAN_ON_PAYLOAD;    
    const char* _mqtt_fan_off_payload = FAN_OFF_PAYLOAD;
    const char* _mqtt_fanSpeed_low_payload = FAN_LOW_PAYLOAD;
    const char* _mqtt_fanSpeed_med_payload = FAN_MED_PAYLOAD;    
    const char* _mqtt_fanSpeed_high_payload = FAN_HIGH_PAYLOAD;   

};


#endif
