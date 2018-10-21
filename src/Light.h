#include "Arduino.h"
#ifndef LightModule_h
#define LightModule_h

#include "Module.h"

#define MQTT_LIGHT_NAME "LIGHT"

#define LIGHT_ON_PAYLOAD "LIGHT_ON"
#define LIGHT_OFF_PAYLOAD "LIGHT_OFF"
#define LIGHT_INVERT_PAYLOAD "INVERT_LIGHT"

class BinaryRelayLight: public ToggleRelayModule
{
  public:
    BinaryRelayLight(IotHandler*, int, bool activeHighRelay = ACTIVE_HIGH_RELAY, long relayActiveTime = RELAY_ACTIVE_TIME, const char* modName = MQTT_LIGHT_NAME);
    BinaryRelayLight(IotHandler*, int, int, bool activeHighRelay = ACTIVE_HIGH_RELAY, long relayActiveTime = RELAY_ACTIVE_TIME, const char* modName = MQTT_LIGHT_NAME);

    void onConnect();                                 //redefine onConnect
    bool triggerAction(String topic, String payload); //redefine triggerAction 
//    void loop();                                      //redefine loop
    
  protected:
    void _setupPins(int, int);  
    void _publishStatus();
    bool _isToggleLight();

    int _light_off_pin;
    int _light_on_pin;

    bool _lightState;
        
    const char* _mqtt_light_on_payload = LIGHT_ON_PAYLOAD;    
    const char* _mqtt_light_off_payload = LIGHT_OFF_PAYLOAD; 
    const char* _mqtt_light_invert_payload = LIGHT_INVERT_PAYLOAD;
};


#endif
