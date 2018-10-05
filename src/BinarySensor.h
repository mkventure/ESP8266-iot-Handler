#include "Arduino.h"
#ifndef BinarySensor_h
#define BinarySensor_h

#include "IotHandler.h"

#define INVERT_SENSOR_STATE false
#define ACTION_TIME 0
#define DEBOUNCE_TIME 500

#define MQTT_BINARYSENSOR_STATUS_TOPIC_SUFFIX "/BINARYSENSOR/STATE"
#define MQTT_BINARYSENSOR_COMMAND_TOPIC_SUFFIX "/BINARYSENSOR/COMMAND"

#define BINARYSENSOR_READ_PAYLOAD "BINARYSENSOR_READ"
#define BINARYSENSOR_ON_PAYLOAD "BINARYSENSOR_ON"
#define BINARYSENSOR_OFF_PAYLOAD "BINARYSENSOR_OFF"

class BinarySensor
{
  public:
    BinarySensor(IotHandler*, int);

    void onConnect();

    void loop();
    bool getState();
    bool getStateChange();
    bool triggerAction(String topic, String payload);

  private:
    void _setupPins(int);
    void _setupHandler(IotHandler*);
    void _publish_sensor_status();

    bool _sensor_read();

    IotHandler* handler;

    int _sensor_pin;
    bool _sensor_state;
    bool _sensor_change;
    
    const int _debounceTime = DEBOUNCE_TIME;
    const int _actionTime = ACTION_TIME;

    char _mqtt_binarysensor_status_topic[40];
    char _mqtt_binarysensor_command_topic[40];
    const char* _mqtt_binarysensor_read_payload = BINARYSENSOR_READ_PAYLOAD;
    const char* _mqtt_binarysensor_on_payload = BINARYSENSOR_ON_PAYLOAD;
    const char* _mqtt_binarysensor_off_payload = BINARYSENSOR_OFF_PAYLOAD;
};


#endif

