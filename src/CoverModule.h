#include "Arduino.h"
#ifndef CoverModule_h
#define CoverModule_h

#include "IotHandler.h"

#define ACTIVE_HIGH_RELAY true
#define COVER_SWITCH_LOGIC true
#define ACTION_TIME 0
#define RELAY_ACTIVE_TIME 500

#define MQTT_COVER_ACTION_TOPIC_SUFFIX "/COVER/SET"
#define MQTT_COVER_STATUS_TOPIC_SUFFIX "/COVER/STATE"
#define MQTT_COVER_COMMAND_TOPIC_SUFFIX "/COVER/COMMAND"

#define COVER_OPEN_PAYLOAD "COVER_OPEN"
#define COVER_CLOSE_PAYLOAD "COVER_CLOSE"
#define COVER_STOP_PAYLOAD "COVER_STOP"

#define MQTT_COVER_SENSE_SIGNATURE bool (*callbackSense)()
//#define MQTT_COVER_STOP_SIGNATURE void (*callbackStop)()

class CoverModule
{
  public:
    CoverModule(IotHandler*, MQTT_COVER_SENSE_SIGNATURE, int, int);
    CoverModule(IotHandler*, MQTT_COVER_SENSE_SIGNATURE, int);
    CoverModule(IotHandler*, int, int);
    CoverModule(IotHandler*, int);

    void onConnect();
    bool triggerAction(String topic, String payload);

    void set_sense_callback(MQTT_COVER_SENSE_SIGNATURE);
//    void set_coverstop_callback(MQTT_COVER_STOP_SIGNATURE);

    void loop();

  private:
    void _setupPins(int, int);
    void _setupHandler(IotHandler*);
    void _publish_cover_status();
    void _toggleRelay(int pin);
    bool _checkSensor();

    MQTT_COVER_SENSE_SIGNATURE;
//    MQTT_COVER_STOP_SIGNATURE;

    IotHandler* handler;

    int _cover_close_pin;
    int _cover_open_pin;

    bool _cover = false;

    const int _actionTime = ACTION_TIME;
    const int _relayActiveTime = RELAY_ACTIVE_TIME;

    char _mqtt_cover_action_topic[40];
    char _mqtt_cover_status_topic[40];
    char _mqtt_cover_command_topic[40];
    const char* _mqtt_cover_open_payload = COVER_OPEN_PAYLOAD;
    const char* _mqtt_cover_close_payload = COVER_CLOSE_PAYLOAD;
    const char* _mqtt_cover_stop_payload = COVER_STOP_PAYLOAD;
};


#endif

