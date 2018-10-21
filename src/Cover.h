#include "Arduino.h"
#ifndef CoverModule_h
#define CoverModule_h

#include "Module.h"

#define MQTT_COVER_NAME "COVER"

#define COVER_OPEN_PAYLOAD "COVER_OPEN"
#define COVER_CLOSE_PAYLOAD "COVER_CLOSE"
#define COVER_STOP_PAYLOAD "COVER_STOP"

#define MQTT_CALLBACK_GETSTATE_SIGNATURE bool (*callbackGetState)()

class Cover: public ToggleRelayModule
{
  public:

    Cover(IotHandler*, int, MQTT_CALLBACK_GETSTATE_SIGNATURE, bool activeHighRelay = ACTIVE_HIGH_RELAY, long relayActiveTime = RELAY_ACTIVE_TIME, const char* modName = MQTT_COVER_NAME);
    Cover(IotHandler*, int, int, MQTT_CALLBACK_GETSTATE_SIGNATURE, bool activeHighRelay = ACTIVE_HIGH_RELAY, long relayActiveTime = RELAY_ACTIVE_TIME, const char* modName = MQTT_COVER_NAME);
    Cover(IotHandler*, int, int, int, MQTT_CALLBACK_GETSTATE_SIGNATURE, bool activeHighRelay = ACTIVE_HIGH_RELAY, long relayActiveTime = RELAY_ACTIVE_TIME, const char* modName = MQTT_COVER_NAME);


    void onConnect();                                 //redefine onConnect
    bool triggerAction(String topic, String payload); //redefine triggerAction
    void loop();                                      //redefine loop

    bool setState(bool);

  protected:
    void _setupPins(int, int, int);
    void _setupPins(int, int);
    void _publishStatus();

    int _open_pin;
    int _close_pin;
    int _stop_pin;

    MQTT_CALLBACK_GETSTATE_SIGNATURE; //varible of callback function to determine current state;

    bool _enableStop;
    bool _coverOpenState;

    const char* _mqtt_cover_open_payload = COVER_OPEN_PAYLOAD;
    const char* _mqtt_cover_close_payload = COVER_CLOSE_PAYLOAD;
    const char* _mqtt_cover_stop_payload = COVER_STOP_PAYLOAD;
};


#endif
