#include "Arduino.h"
#ifndef BinarySensor_h
#define BinarySensor_h

#include "Module.h"

#define DEBOUNCE_TIME 500

#define MQTT_BINARYSENSOR_NAME "BINARYSENSOR"
//#define MQTT_BINARYSENSOR_COMMAND_TOPIC_SUFFIX "BINARYSENSOR"

#define BINARYSENSOR_READ_PAYLOAD "BINARYSENSOR_READ"
#define BINARYSENSOR_ON_PAYLOAD "BINARYSENSOR_ON"
#define BINARYSENSOR_OFF_PAYLOAD "BINARYSENSOR_OFF"

//Driven Binary Sensor implements MQTT protocols for communication but state is driven externally by program functions
class BinarySensor : public Module
{
  public:
    BinarySensor(IotHandler*, const char* modName = MQTT_BINARYSENSOR_NAME);

    void onConnect(); //redefine onConnect
    bool triggerAction(String topic, String payload); //redefine triggerAction 
    
    bool setState(bool);
    bool getState();
    unsigned long getChangeTime();

  protected:
    void _publishStatus();
    
    unsigned long _changeTime;
    bool _sensorState;
    const char* _mqtt_binarysensor_read_payload = BINARYSENSOR_READ_PAYLOAD;
    const char* _mqtt_binarysensor_on_payload = BINARYSENSOR_ON_PAYLOAD;
    const char* _mqtt_binarysensor_off_payload = BINARYSENSOR_OFF_PAYLOAD;    
};

//Pin Binary Sensor implements MQTT protocols for communcation and state is driven internally by a specific hardware pin state
class BinarySensor_Pin : public BinarySensor
{
  public:
    BinarySensor_Pin(IotHandler*, int, const char* modName = MQTT_BINARYSENSOR_NAME, int debounceTime = DEBOUNCE_TIME);
    bool getStateChange();
    void loop(); //redefine loop to read sensor on each loop.

  protected:
    void _setupPins(int);    
    bool _sensorRead();

    bool _sensorChange = false;
    int _sensorPin;
    int _debounceTime;
};

#endif
