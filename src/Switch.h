#ifndef SwitchModule_h
#define SwitchModule_h

#include "IotHandler.h"
#include "SwitchInterface.h"
#include "Arduino.h"

#define SWITCH_FUNCTION_CALLBACK void (*callback_on_function)()
#define SWITCH_FUNCTION_CALLBACK void (*callback_off_function)()


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

class Switch_Function : public Switch
{
  public:
    Switch_Function(IotHandler* handler, const char* mqtt_object_id, void (*callback_on_function)(), void (*callback_off_function)(), bool state, const char* mqtt_component = "switch")
      : Switch(handler, mqtt_object_id, state, mqtt_component), switchFunction(callback_on_function, callback_off_function) {};

    virtual bool setState(bool state);
    bool setStateFor(bool state, unsigned long time) {
      return switchFunction.setStateFor(state, time);
    }
    bool getState() {
      return switchFunction.getState();
    }

  protected:
    SwitchInterface_Function switchFunction;

    void _loop() {
      switchFunction.switch_loop();
    }
};

class Switch_Pin : public Switch
{
  public:
    Switch_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, bool state, bool invert, const char* mqtt_component = "switch")
      : Switch(handler, mqtt_object_id, mqtt_component), switchPin(pin, state, invert) {};
    Switch_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, const char* mqtt_component = "switch")
      : Switch(handler, mqtt_object_id, mqtt_component), switchPin(pin) {};

    virtual bool setState(bool state);
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

class ToggleSwitch_Function : public Switch_Function
{
  public:
    ToggleSwitch_Function(IotHandler* handler, const char* mqtt_object_id, void (*callback_on_function)(), void (*callback_off_function)(), bool state, int toggleTime, const char* mqtt_component = "switch")
      : Switch_Function(handler, mqtt_object_id, callback_on_function, callback_off_function, state, mqtt_component), _toggleTime(toggleTime) {};

    bool setState(bool state) {
      return setStateFor(state, _toggleTime);
    }

  protected:
    int _toggleTime;
};


class ToggleSwitch_Pin : public Switch_Pin
{
  public:
    ToggleSwitch_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, int toggleTime, bool state, bool invert, const char* mqtt_component = "switch")
      : Switch_Pin(handler, mqtt_object_id, pin, state, invert, mqtt_component), _toggleTime(toggleTime) {};
    ToggleSwitch_Pin(IotHandler* handler, const char* mqtt_object_id, int pin, int toggleTime, const char* mqtt_component = "switch")
      : Switch_Pin(handler, mqtt_object_id, pin, mqtt_component), _toggleTime(toggleTime) {};

    bool setState(bool state) {
      return setStateFor(state, _toggleTime);
    }

  protected:
    int _toggleTime;
};
#endif
