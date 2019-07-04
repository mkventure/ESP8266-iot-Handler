#ifndef LightModule_h
#define LightModule_h

#include "IotHandler.h"
#include "SwitchInterface.h"
#include "Arduino.h"

#define STATE_ON_PAYLOAD "ON"
#define STATE_OFF_PAYLOAD "OFF"
#define STATE_INVERT_PAYLOAD "INVERT"

class BinaryLight : public ActionModule
{
  public:
    BinaryLight(IotHandler*, const char*, const char* = "light");
    BinaryLight(IotHandler*, const char*, bool, const char* = "light");

    bool getState() {
      return _lightState;
    }
    bool setState(bool);

  protected:
    virtual bool _handleAction(String topic, String payload);
    virtual void _publishStatus();

    //    virtual void _onConnect();
    //    virtual void _setup();
    //    virtual void _loop() {};

    virtual bool setHWState(bool) = 0;

  private:
    bool _lightState;

    const char* _mqtt_state_on_payload = STATE_ON_PAYLOAD;
    const char* _mqtt_state_off_payload = STATE_OFF_PAYLOAD;
};

class BinaryLight_TogglePin : BinaryLight
{
  public:
    BinaryLight_TogglePin(IotHandler*, const char*, int pin, bool initital, bool state, unsigned long toggle_time = TOGGLE_TIME, const char* = "light");
    BinaryLight_TogglePin(IotHandler*, const char*, int pin, bool initital, const char* = "light");
    //    bool setState(bool);

  protected:
    virtual bool _handleAction(String topic, String payload);
    //    virtual void _onConnect();
    //    virtual void _setup();
    void _loop() {
      lightControl.switch_loop();
    }
    bool setHWState(bool);
  private:
    ToggleInterface_Pin lightControl;
    const char* _mqtt_state_invert_payload = STATE_INVERT_PAYLOAD;

};

class BinaryLight_Pin : BinaryLight
{
  public:
    BinaryLight_Pin(IotHandler*, const char*, int pin, bool initital, bool invert, const char* = "light");
    //    bool setState(bool);

  protected:
//    virtual bool _handleAction(String topic, String payload);
    //    virtual void _onConnect();
    //        void _setup();
    void _loop() {
      lightControl.switch_loop();
    }
    bool setHWState(bool);
  private:
    SwitchInterface_Pin lightControl;

};

class BriLight : public BinaryLight
{
  public:
    BriLight(IotHandler*, const char*, const char* = "light");
    byte getBri() {
      return _bri;
    }
    virtual bool setBri(byte) = 0;

  protected:
    virtual bool _handleAction(String topic, String payload);
    virtual void _onConnect();
    virtual void _setup();
    virtual void _loop();

    virtual void _publishStatus();

  private:
    byte _bri = 0;

    char _mqtt_bri_command_topic[_MQTT_TOPIC_SIZE];
    char _mqtt_bri_status_topic[_MQTT_TOPIC_SIZE];
};

class RGBLight : public BriLight {};

#endif

