#include "SwitchInterface.h"

SwitchInterface::SwitchInterface(bool initialState, bool invertOutput)
  : _invert(invertOutput), _switchState(initialState ^ invertOutput)
{
  //  Serial.printf("setinit:\tState:%d\tInvert:%d\tA^B:%d\n", initialState, _invert, initialState ^ _invert);
  Serial.println("Start SwitchInterface");
}

bool SwitchInterface::setState(bool state) {
//  Serial.printf("Try Change (%d);\tVirtual: %d\tHW:\%d\tInvert:%d\n", state, getState(), getRawState(), _invert);
  if (getState() != state) {
    _timerFlag = false;
    //    Serial.printf("setstate:\tState:%d\tInvert:%d\tA^B:%d\n", state, _invert, state ^ _invert);
    _switchState = state ^ _invert;
    Serial.printf("State Changed;\tVirtual: %d\tHW:\%d\tInvert:%d\n", getState(), getRawState(), _invert);
    return true;
  }
//  Serial.printf("State Not Changed;\tVirtual: %d\tHW:\%d\tInvert:%d\n", getState(), getRawState(), _invert);
  return false;
}

void SwitchInterface::switch_loop() {
  if (_timerFlag && millis() >= _timer) {
//    Serial.printf("Reset. Current Time: %d\t Timer: %d\n", millis(), _timer);
    setState(!getState());
  }
}

bool SwitchInterface::setStateFor(bool state, unsigned long timer) {
  if (getState() != state) {
    _timer = millis() + timer;
//    Serial.printf("Toggle. Current Time: %d\t Timer: %d\n", millis(), _timer);
    setState(state);
    _timerFlag = true;
    return true;
  }
  return false;
}


SwitchInterface_Pin::SwitchInterface_Pin(int pin, bool initialState, bool invertOutput)
  : SwitchInterface(initialState, invertOutput), _pin(pin)
{
  Serial.println("Start SwitchInterface_Pin");
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, getRawState());
}


bool SwitchInterface_Pin::setState(bool state) {
  if (SwitchInterface::setState(state)) {
    digitalWrite(_pin, getRawState());
    return true;
  }
  return false;
}


SwitchInterface_Function::SwitchInterface_Function(void (*callback_on_function)(), void (*callback_off_function)(), bool initialState, bool invertOutput)
  : SwitchInterface(initialState, invertOutput), callback_on_function(callback_on_function), callback_off_function(callback_off_function)
{
  Serial.println("Start SwitchInterface_Function");
}

bool SwitchInterface_Function::setState(bool state) {
  if (SwitchInterface::setState(state)) {
    if(state) {
      callback_on_function;
    }
    else {
      callback_off_function;
    }
    return true;
  }
  return false;
}

ToggleInterface::ToggleInterface(bool state, unsigned long toggle_time)
  : SwitchInterface(state, state), _toggleState(!state), _toggle_time(toggle_time)
{
  Serial.println("Start ToggleInterface");
}

bool ToggleInterface::toggle() {
  if (setStateFor(_toggleState, _toggle_time)) {
    Serial.println("\tToggle succeed. Publish");
    return true;
  }
  Serial.println("\tToggle failed. No Publish");
  return false;
}

ToggleInterface_Pin::ToggleInterface_Pin(int pin, bool restingState, unsigned long toggle_time)
  : ToggleInterface(restingState, toggle_time), _pin(pin)
{
  Serial.println("Start ToggleInterface_Pin");
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, getRawState());
}

bool ToggleInterface_Pin::setState(bool restingState) {
  if (ToggleInterface::setState(restingState)) {
    digitalWrite(_pin, getRawState());
    return true;
  }
  return false;
}

ToggleInterface_Function::ToggleInterface_Function(void (*callback_on_function)(), void (*callback_off_function)(), bool restingState, unsigned long toggle_time)
  : ToggleInterface(restingState, toggle_time), callback_on_function(callback_on_function), callback_off_function(callback_off_function)
{
  Serial.println("Start SwitchInterface_Function");
}

bool ToggleInterface_Function::setState(bool restingState) {
  if (ToggleInterface::setState(restingState)) {
    if(getRawState()) {
      callback_on_function();
    }
    else {
      callback_off_function();
    }
    return true;
  }
  return false;
}
