#ifndef FanModule_h
#define FanModule_h

#include "IotHandler.h"
#include "SwitchInterface.h"
#include "Arduino.h"

#define FAN_ON_DELAY 50

#define MQTT_FAN_SPEED_NAME "speed"

#define FAN_ON_PAYLOAD "ON"
#define FAN_OFF_PAYLOAD "OFF"
#define FAN_LOW_PAYLOAD "low"
#define FAN_MED_PAYLOAD "medium"
#define FAN_HIGH_PAYLOAD "high"

enum FanSpeed {
  FAN_HIGH,
  FAN_MED,
  FAN_LOW
};

class Fan : public ActionModule
{
  public:
    Fan(IotHandler*, const char*, const char* = "fan");

    bool getState() {
      return _fanState;
    }
    FanSpeed getSpeed() {
      return _fanSpeed;
    }

    bool setState(bool state, FanSpeed speed);

  protected:
    bool _handleAction(String topic, String payload);
    void _onConnect();
    void _setup();
    virtual void _loop();

    void _publishStatus();
    void _publishFanStatus();
    void _publishSpeedStatus();
    
    virtual bool _setOff() = 0;
    virtual bool _setLow() = 0;
    virtual bool _setMed() = 0;
    virtual bool _setHigh() = 0;

  private:
    bool _fanState;
    enum FanSpeed _fanSpeed;
    
    bool _fan_onDelay_bool = true;
    const unsigned long _fan_onDelay = FAN_ON_DELAY;
    bool _delayOnFlag = false;
    unsigned long _fan_onTime;

    char _mqtt_fanspeed_command_topic[_MQTT_TOPIC_SIZE];
    char _mqtt_fanspeed_status_topic[_MQTT_TOPIC_SIZE];

    const char* _mqtt_state_on_payload = STATE_ON_PAYLOAD;
    const char* _mqtt_state_off_payload = STATE_OFF_PAYLOAD;    
    const char* _mqtt_fanSpeed_low_payload = FAN_LOW_PAYLOAD;
    const char* _mqtt_fanSpeed_med_payload = FAN_MED_PAYLOAD;
    const char* _mqtt_fanSpeed_high_payload = FAN_HIGH_PAYLOAD;
};


class Fan_Pin : public Fan
{
  public:
    Fan_Pin(IotHandler*, const char*, int, int, int, int, bool restingState, long relayActiveTime = TOGGLE_TIME, const char* = "fan");
    Fan_Pin(IotHandler*, const char*, int, int, int, int, const char* = "fan");

  protected:
    void _loop() {
      Fan::_loop();
      _off_toggle.switch_loop();
      _low_toggle.switch_loop();
      _med_toggle.switch_loop();
      _high_toggle.switch_loop();
    }

  private:
    ToggleInterface_Pin _off_toggle;
    ToggleInterface_Pin _low_toggle;
    ToggleInterface_Pin _med_toggle;
    ToggleInterface_Pin _high_toggle;

    bool _setOff() {
//      Serial.println("Toggle Off");
      return _off_toggle.toggle();
    }
    bool _setLow() {
//      Serial.println("Toggle Low");
      return _low_toggle.toggle();
    }
    bool _setMed() {
//      Serial.println("Toggle Med");
      return _med_toggle.toggle();
    }
    bool _setHigh() {
//      Serial.println("Toggle High");
      return _high_toggle.toggle();
    }
};

#endif
