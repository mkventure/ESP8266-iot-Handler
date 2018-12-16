#ifndef SwitchModule_h
#define SwitchModule_h

#include "IotHandler.h"
#include "SwitchInterface.h"
#include "Arduino.h"



class Switch : public ActionModule
{
  public:
    Switch(IotHandler*, const char*, bool state, bool invert, const char* = "switch");
    Switch(IotHandler*, const char*, const char* = "switch");
    
    virtual bool setState(bool) = 0;
    virtual bool setStateFor(bool, unsigned long) = 0;
    virtual bool getState() = 0;

  protected:
    bool _handleAction(String topic, String payload);
    void _publishStatus();
    //    virtual void _setup();
    //    virtual void _loop();

  private:
    const char* _mqtt_state_on_payload = STATE_ON_PAYLOAD;
    const char* _mqtt_state_off_payload = STATE_OFF_PAYLOAD;
};

class Switch_Pin : public Switch
{
  public:
    Switch_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, bool state, bool invert, const char* mqtt_component = "switch")
      : Switch(handler, mqtt_object_id, mqtt_component), switchPin(pin, state, invert) {};
    Switch_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, const char* mqtt_component = "switch")
      : Switch(handler, mqtt_object_id, mqtt_component), switchPin(pin) {};

    bool setState(bool state);
    bool setStateFor(bool state, unsigned long time) {
      return switchPin.setStateFor(state, time);
    }
    bool getState() {
      return switchPin.getState();
    }

  protected:
    SwitchInterface_Pin switchPin;
    
    void _loop() {
      switchPin.switch_loop();
    }
};



#endif
