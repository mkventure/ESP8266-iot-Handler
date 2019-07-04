#ifndef SwitchInterface2_h
#define SwitchInterface2_h

#define RESTING_STATE false
#define TOGGLE_TIME 100

#include "Arduino.h"



class SwitchInterface
{
  public:
    SwitchInterface(bool initialState = false, bool invertOutput = false);
    virtual bool setState(bool);
    bool setStateFor(bool, unsigned long);
    bool getState() {
      //      Serial.printf("getstate:\tState:%d\tInvert:%d\tA^B:%d\n", _switchState, _invert, _switchState ^ _invert);
      return _switchState ^ _invert;
    }
    void switch_loop();

  protected:
    bool getRawState() {
      return _switchState; //gets state of hw ignoring invert
    }

  private:
    bool _invert;
    bool _timerFlag = false;
    unsigned long _timer;
    bool _switchState; //actual state of the HW. _invert is accounted for at get and set state functions
};


class SwitchInterface_Pin: public SwitchInterface
{
  public:
    SwitchInterface_Pin(int pin, bool initialState = false, bool invertOutput = false);
    bool setState(bool);
  private:
    int _pin;
};

class SwitchInterface_Function: public SwitchInterface
{
  public:
    SwitchInterface_Function(void (*callback_on_function)(), void (*callback_off_function)(), bool initialState = false, bool invertOutput = false);
    bool setState(bool);
  private:    
    void (*callback_on_function)();
    void (*callback_off_function)();
};

class ToggleInterface: protected SwitchInterface
{
  public:
    using SwitchInterface::switch_loop;
    ToggleInterface(bool restingState = RESTING_STATE, unsigned long toggle_time = TOGGLE_TIME);
    bool toggle();

  private:
    bool _toggleState;
    unsigned long _toggle_time;
};

class ToggleInterface_Pin : public ToggleInterface
{
  public:
    ToggleInterface_Pin(int pin, bool restingState = RESTING_STATE, unsigned long toggle_time = TOGGLE_TIME);
    bool setState(bool);
  private:
    int _pin;
};


class ToggleInterface_Function: public ToggleInterface
{
  public:
    ToggleInterface_Function(void (*callback_on_function)(), void (*callback_off_function)(), bool restingState = RESTING_STATE, unsigned long toggle_time = TOGGLE_TIME);
    bool setState(bool);
  private:    
    void (*callback_on_function)();
    void (*callback_off_function)();
};

#endif
