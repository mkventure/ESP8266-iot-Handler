#include "Arduino.h"
#ifndef SwitchModule_h
#define SwitchModule_h

#include "Module.h"

#define MQTT_SWITCH_NAME "SWITCH"

#define SWITCH_ON_PAYLOAD "SWITCH_ON"
#define SWITCH_OFF_PAYLOAD "SWITCH_OFF"

class BinarySwitch: public ActionModule
{
  public:
    BinarySwitch(IotHandler*, const char* modName = MQTT_SWITCH_NAME, bool state = false);

    void onConnect();                                 //redefine onConnect
    bool triggerAction(String topic, String payload); //redefine triggerAction
    void loop();                                      //redefine loop
	bool getState();
    virtual bool setState(bool);
    bool setStateFor(bool, unsigned long);

  protected:
    void _publishStatus();
    
    bool _timerFlag = false;
    unsigned long _timer;

    bool _switchState;

    const char* _mqtt_switch_on_payload = SWITCH_ON_PAYLOAD;
    const char* _mqtt_switch_off_payload = SWITCH_OFF_PAYLOAD;
};


class BinarySwitch_Pin: public BinarySwitch
{
  public:
    BinarySwitch_Pin(IotHandler*, int, const char* modName = MQTT_SWITCH_NAME, bool state = false);
    bool setState(bool) override;
    
  protected:
    void _setupPins(int);

    int _switch_pin;
};

#endif
