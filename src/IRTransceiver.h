#include "Arduino.h"
#ifndef IRTransceiver_h
#define IRTransceiver_h

#include "Module.h"
#include "Switch.h"

//#define ARDUINO ARDUINO
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#define MQTT_TRANSMITTER_NAME "IR_TRANSMITTER"
#define MQTT_RECEIVER_NAME "IR_RECEIVER"

class IRReceiver: public BinarySwitch
{
  public:
    IRReceiver(IotHandler*, int, const char* modName = MQTT_RECEIVER_NAME, bool enableRecv = false);

    void loop();                                      //redefine loop
    void onConnect();                                 //redefine onconnect and don't call parent
    bool setState(bool) override;

  protected:
    IRrecv irrecv;
    decode_results _results;

    void _publish_IR();
    void _fullprint();
    String _commandPrint(const decode_results *results);
};

class IRTransmitter: public ActionModule
{
  public:
    IRTransmitter(IotHandler*, int, const char* modName = MQTT_TRANSMITTER_NAME);

    bool triggerAction(String topic, String payload); //redefine triggerAction

  protected:
    IRsend irsend;

    String _lastCommand = "N/A";
    bool _lastCommandSuccess = false;

    void _publishStatus();                                 //redefine publish status
    bool parseActionSignal(String payload);
    bool sendIRCode(int const ir_type, uint64_t const code, char const * code_str, uint16_t bits, uint16_t repeat);
};

/*
   Send a MQTT message to the topic 'ir_server/send' using the following
   format (Order is important):
     protocol_num,hexcode  e.g. 7,E0E09966 which is Samsung(7), Power On code,
                                default bit size, default nr. of repeats.
     protocol_num,hexcode,bits  e.g. 4,f50,12 which is Sony(4), Power Off code,
                                 12 bits & default nr. of repeats.
     protocol_num,hexcode,bits,repeats  e.g. 19,C1A2E21D,0,8 which is
                                        Sherwood(19), Vol Up, default bit size &
                                        repeated 8 times.
     30,frequency,raw_string  e.g. 30,38000,9000,4500,500,1500,500,750,500,750
                               Raw (30) @ 38kHz with a raw code of "9000,4500,500,1500,500,750,500,750"
     31,code_string  e.g. 31,40000,1,1,96,24,24,24,48,24,24,24,24,24,48,24,24,24,24,24,48,24,24,24,24,24,24,24,24,1058
                          GlobalCache (31) & "40000,1,1,96,..." (Sony Vol Up)
     25,Rrepeats,hex_code_string  e.g. 25,R1,0000,006E,0022,0002,0155,00AA,0015,0040,0015,0040,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0040,0015,0040,0015,0015,0015,0040,0015,0015,0015,0015,0015,0015,0015,0040,0015,0015,0015,0015,0015,0040,0015,0040,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0015,0040,0015,0015,0015,0015,0015,0040,0015,0040,0015,0040,0015,0040,0015,0040,0015,0640,0155,0055,0015,0E40
                                 Pronto (25), 1 repeat, & "0000 006E 0022 0002 ..." (Sherwood Amp Tape Input)
     ac_protocol_num,really_long_hexcode  e.g. 18,190B8050000000E0190B8070000010F0
                             Kelvinator (18) Air Con on, Low Fan, 25 deg etc.
                             NOTE: Ensure you zero-pad to the correct number of
                                   digits for the bit/byte size you want to send
                                   as some A/C units have units have different
                                   sized messages. e.g. Fujitsu A/C units.
     In short:
       No spaces after/before commas.
       Values are comma separated.
       The first value is always in Decimal.
       For simple protocols, the next value (hexcode) is always hexadecimal.
       The optional bit size is in decimal.

*/

#endif

