#include "Arduino.h"
#ifndef BinarySensor_h
#define BinarySensor_h

#include "IotHandler.h"
#define DEBOUNCE_TIME 500
#define MQTT_COMPONENT_BINARYSENSOR "binary_sensor"

//https://www.home-assistant.io/components/binary_sensor/


class BinarySensor : public Module
{
  public:
    BinarySensor(IotHandler*, const char*, const char* = "None");
    BinarySensor(IotHandler*, const char*, int, const char* = "None");
    bool getState() {
      return _sensorState;
    }

  protected:
    bool readState();
    virtual bool readHWState() = 0;
    void _publishStatus();
    void _loop();

    bool _sensorState;

  private:
    int _debounceTime;

    const char* _mqtt_state_on_payload = STATE_ON_PAYLOAD;
    const char* _mqtt_state_off_payload = STATE_OFF_PAYLOAD;
};


class BinarySensor_Pin : public BinarySensor
{
  public:
    BinarySensor_Pin(IotHandler*, const char*, int, const char* = "None");
    BinarySensor_Pin(IotHandler*, const char*, int, int, const char* = "None");

  protected:
    bool readHWState();
    int _sensorPin;
};

#endif

