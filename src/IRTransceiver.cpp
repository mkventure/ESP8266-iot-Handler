#include "IRTransceiver.h"
/*
   https://github.com/markszabo/IRremoteESP8266
   Copyright Notice:
     Code for this has been borrowed from lots of other OpenSource projects &
     resources. I'm *NOT* claiming complete Copyright ownership of all the code.
     Likewise, feel free to borrow from this as much as you want.
*/

uint64_t getUInt64fromHex(char const *str) {
  uint64_t result = 0;
  uint16_t offset = 0;
  // Skip any leading '0x' or '0X' prefix.
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    offset = 2;
  for (; isxdigit((unsigned char)str[offset]); offset++) {
    char c = str[offset];
    result *= 16;
    if (isdigit(c)) /* '0' .. '9' */
      result += c - '0';
    else if (isupper(c)) /* 'A' .. 'F' */
      result += c - 'A' + 10;
    else /* 'a' .. 'f'*/
      result += c - 'a' + 10;
  }
  return result;
}

void printLL(uint64_t n, uint8_t base)
{
  unsigned char buf[16 * sizeof(long)];
  unsigned int i = 0;

  if (n == 0)
  {
    Serial.print((char)'0');
    return;
  }
  while (n > 0)
  {
    buf[i++] = n % base;
    n /= base;
  }
  for (; i > 0; i--)
    Serial.print((char) (buf[i - 1] < 10 ?
                         '0' + buf[i - 1] :
                         'A' + buf[i - 1] - 10));
}

IRReceiver::IRReceiver(IotHandler* handler, int pin, const char* modName, bool enableRecv)
  : BinarySwitch(handler, modName, enableRecv), irrecv(pin)
{
  if (getState()) {
    irrecv.enableIRIn();
  }
  else {
    irrecv.disableIRIn();
  }
}

void IRReceiver::_publish_IR() {
  String r = _commandPrint(&_results);
  Serial.print("Publishing IR Recv Command: ");
  Serial.print(_mqtt_status_topic);
  Serial.print("\t");
  handler->client.publish(_mqtt_command_topic, r.c_str(), true);
  Serial.println(r);
}

void IRReceiver::loop() {
  BinarySwitch:: loop();
  if (getState() && irrecv.decode(&_results)) {
    //filter out 'unknown' IR reads, these are most likely garbage. And filter out the repeats.
    if (_results.decode_type > 0 && resultToHexidecimal(&_results) != "FFFFFFFFFFFFFFFF" ) {
      //    Serial.println(resultToHumanReadableBasic(&_results));
      _publish_IR();
    }
    irrecv.resume();  // Receive the next value
  }
}

void IRReceiver::onConnect() {
  //do not call parent functions. Do not need to listen to /command
  Serial.print("Subscribing to ");
  Serial.print(_mqtt_action_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_action_topic);
}

String IRReceiver::_commandPrint(const decode_results *results) {
  String output = "Command Received: ";
  output += results->decode_type;
  output += ",";
  output += resultToHexidecimal(results);
  output += ",";
  output += uint64ToString(results->bits);
  //  output += ",";
  //  output += results->repeat;

  return output;
}

bool IRReceiver::setState(bool state) {
  if (BinarySwitch::setState(state)) {
    if (getState()) {
      irrecv.enableIRIn();
    }
    else {
      irrecv.disableIRIn();
    }
    return true;
  }
  return false;
}

/******************************************

***********************************************/

IRTransmitter::IRTransmitter(IotHandler* handler, int pin, const char* modName)
  : ActionModule(handler, modName), irsend(pin)
{
  irsend.begin();
}

void IRTransmitter::_publishStatus() {
  String s = "Command Parse Success: ";
  s += _lastCommand;
  s += "  ";
  s += _lastCommand;
  handler->client.publish(_mqtt_status_topic, s.c_str(), true);
  Serial.printf("Parse Successful?: %X; Transmitted %s to %s\n", _lastCommandSuccess, _lastCommand.c_str(), _mqtt_status_topic);
}

bool IRTransmitter::triggerAction(String topic, String payload) {
  if (topic == _mqtt_action_topic && payload) {
    _lastCommandSuccess = parseActionSignal(payload);
    _lastCommand = payload;
    _publishStatus();
  }
  else {
    return false;
  }
  return true;
}

bool IRTransmitter::parseActionSignal(String payload) {
  Serial.print("Parsed: ");

  char* tok_ptr;
  uint64_t code = 0;
  uint16_t nbits = 0;
  uint16_t repeat = 0;

  // Make a copy of the callback string as strtok destroys it.
  char* callback_c_str = strdup(payload.c_str());

  // Get the numeric protocol type.
  int ir_type = strtoul(strtok_r(callback_c_str, ", ", &tok_ptr), NULL, 10);
  char* next = strtok_r(NULL, ", ", &tok_ptr);

  // If there is unparsed string left, try to convert it assuming it's hex.
  if (next != NULL) {
    code = getUInt64fromHex(next);
    next = strtok_r(NULL, ", ", &tok_ptr);
  } else {
    // We require at least two value in the string. Give up.
    Serial.println("Failed.");
    return false;
  }

  // If there is still string left, assume it is the bit size.
  if (next != NULL) {
    nbits = atoi(next);
    next = strtok_r(NULL, ", ", &tok_ptr);
  }
  // If there is still string left, assume it is the repeat count.
  if (next != NULL)
    repeat = atoi(next);

  free(callback_c_str);

  Serial.print("\tIRType: ");
  Serial.print(ir_type);
  Serial.print("\tCode: ");
  //  Serial.print(uint64ToHex(code));
  printLL(code, 16);
  Serial.print("\tNBits: ");
  Serial.print(nbits);
  Serial.print("\tRepeat: ");
  Serial.println(repeat);

  return sendIRCode(
           ir_type, code,
           payload.substring(payload.indexOf(", ") + 1).c_str(),
           nbits, repeat);
}


bool IRTransmitter::sendIRCode(int const ir_type, uint64_t const code, char const * code_str, uint16_t bits, uint16_t repeat) {
  // Create a pseudo-lock so we don't try to send two codes at the same time.

  static bool ir_lock = false;

  while (ir_lock)
    delay(20);
  ir_lock = true;

  bool success = true;  // Assume success.

  // send the IR message.
  switch (ir_type) {
#if SEND_RC5
    case RC5:  // 1
      if (bits == 0)
        bits = kRC5Bits;
      irsend.sendRC5(code, bits, repeat);
      break;
#endif
#if SEND_RC6
    case RC6:  // 2
      if (bits == 0)
        bits = kRC6Mode0Bits;
      irsend.sendRC6(code, bits, repeat);
      break;
#endif
#if SEND_NEC
    case NEC:  // 3
      if (bits == 0)
        bits = kNECBits;
      irsend.sendNEC(code, bits, repeat);
      break;
#endif
#if SEND_SONY
    case SONY:  // 4
      if (bits == 0)
        bits = kSony12Bits;
      repeat = std::max(repeat, kSonyMinRepeat);
      irsend.sendSony(code, bits, repeat);
      break;
#endif
#if SEND_PANASONIC
    case PANASONIC:  // 5
      if (bits == 0)
        bits = kPanasonicBits;
      irsend.sendPanasonic64(code, bits, repeat);
      break;
#endif
#if SEND_JVC
    case JVC:  // 6
      if (bits == 0)
        bits = kJvcBits;
      irsend.sendJVC(code, bits, repeat);
      break;
#endif
#if SEND_SAMSUNG
    case SAMSUNG:  // 7
      if (bits == 0)
        bits = kSamsungBits;
      irsend.sendSAMSUNG(code, bits, repeat);
      break;
#endif
#if SEND_WHYNTER
    case WHYNTER:  // 8
      if (bits == 0)
        bits = kWhynterBits;
      irsend.sendWhynter(code, bits, repeat);
      break;
#endif
#if SEND_AIWA_RC_T501
    case AIWA_RC_T501:  // 9
      if (bits == 0)
        bits = kAiwaRcT501Bits;
      repeat = std::max(repeat, kAiwaRcT501MinRepeats);
      irsend.sendAiwaRCT501(code, bits, repeat);
      break;
#endif
#if SEND_LG
    case LG:  // 10
      if (bits == 0)
        bits = kLgBits;
      irsend.sendLG(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI
    case MITSUBISHI:  // 12
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      irsend.sendMitsubishi(code, bits, repeat);
      break;
#endif
#if SEND_DISH
    case DISH:  // 13
      if (bits == 0)
        bits = kDishBits;
      repeat = std::max(repeat, kDishMinRepeat);
      irsend.sendDISH(code, bits, repeat);
      break;
#endif
#if SEND_SHARP
    case SHARP:  // 14
      if (bits == 0)
        bits = kSharpBits;
      irsend.sendSharpRaw(code, bits, repeat);
      break;
#endif
#if SEND_COOLIX
    case COOLIX:  // 15
      if (bits == 0)
        bits = kCoolixBits;
      irsend.sendCOOLIX(code, bits, repeat);
      break;
#endif
      //    case DAIKIN:  // 16
      //    case KELVINATOR:  // 18
      //    case MITSUBISHI_AC:  // 20
      //    case GREE:  // 24
      //    case ARGO:  // 27
      //    case TROTEC:  // 28
      //    case TOSHIBA_AC:  // 32
      //    case FUJITSU_AC:  // 33
      //    case HAIER_AC:  // 38
      //    case HAIER_AC_YRW02:  // 44
      //    case HITACHI_AC:  // 40
      //    case HITACHI_AC1:  // 41
      //    case HITACHI_AC2:  // 42
      //    case WHIRLPOOL_AC:  // 45
      //    case SAMSUNG_AC:  // 46
      //    case ELECTRA_AC:  // 48
      //    case PANASONIC_AC:  // 49
      //      success = parseStringAndSendAirCon(ir_type, code_str);
      //      break;
#if SEND_DENON
    case DENON:  // 17
      if (bits == 0)
        bits = DENON_BITS;
      irsend.sendDenon(code, bits, repeat);
      break;
#endif
#if SEND_SHERWOOD
    case SHERWOOD:  // 19
      if (bits == 0)
        bits = kSherwoodBits;
      repeat = std::max(repeat, kSherwoodMinRepeat);
      irsend.sendSherwood(code, bits, repeat);
      break;
#endif
#if SEND_RCMM
    case RCMM:  // 21
      if (bits == 0)
        bits = kRCMMBits;
      irsend.sendRCMM(code, bits, repeat);
      break;
#endif
#if SEND_SANYO
    case SANYO_LC7461:  // 22
      if (bits == 0)
        bits = kSanyoLC7461Bits;
      irsend.sendSanyoLC7461(code, bits, repeat);
      break;
#endif
#if SEND_RC5
    case RC5X:  // 23
      if (bits == 0)
        bits = kRC5XBits;
      irsend.sendRC5(code, bits, repeat);
      break;
#endif
      //#if SEND_PRONTO
      //    case PRONTO:  // 25
      //      success = parseStringAndSendPronto(code_str, repeat);
      //      break;
      //#endif
#if SEND_NIKAI
    case NIKAI:  // 29
      if (bits == 0)
        bits = kNikaiBits;
      irsend.sendNikai(code, bits, repeat);
      break;
#endif
      //#if SEND_RAW
      //    case RAW:  // 30
      //      success = parseStringAndSendRaw(code_str);
      //      break;
      //#endif
      //#if SEND_GLOBALCACHE
      //    case GLOBALCACHE:  // 31
      //      success = parseStringAndSendGC(code_str);
      //      break;
      //#endif
#if SEND_MIDEA
    case MIDEA:  // 34
      if (bits == 0)
        bits = kMideaBits;
      irsend.sendMidea(code, bits, repeat);
      break;
#endif
#if SEND_MAGIQUEST
    case MAGIQUEST:  // 35
      if (bits == 0)
        bits = kMagiquestBits;
      irsend.sendMagiQuest(code, bits, repeat);
      break;
#endif
#if SEND_LASERTAG
    case LASERTAG:  // 36
      if (bits == 0)
        bits = kLasertagBits;
      irsend.sendLasertag(code, bits, repeat);
      break;
#endif
#if SEND_CARRIER_AC
    case CARRIER_AC:  // 37
      if (bits == 0)
        bits = kCarrierAcBits;
      irsend.sendCarrierAC(code, bits, repeat);
      break;
#endif
#if SEND_MITSUBISHI2
    case MITSUBISHI2:  // 39
      if (bits == 0)
        bits = kMitsubishiBits;
      repeat = std::max(repeat, kMitsubishiMinRepeat);
      irsend.sendMitsubishi2(code, bits, repeat);
      break;
#endif
#if SEND_GICABLE
    case GICABLE:  // 43
      if (bits == 0)
        bits = kGicableBits;
      repeat = std::max(repeat, kGicableMinRepeat);
      irsend.sendGICable(code, bits, repeat);
      break;
#endif
#if SEND_LUTRON
    case LUTRON:  // 47
      if (bits == 0)
        bits = kLutronBits;
      irsend.sendLutron(code, bits, repeat);
      break;
#endif
    default:
      // If we got here, we didn't know how to send it.
      success = false;
  }
  // Release the lock.
  ir_lock = false;

  return success;
}


