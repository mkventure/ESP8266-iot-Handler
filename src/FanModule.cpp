#include "FanModule.h"

FanModule::FanModule(IotHandler* handler, int fan_off_pin, int fan_low_pin, int fan_med_pin, int fan_high_pin) {
  this->handler = handler;

  const char *fan_action_topic_suffix = MQTT_FAN_ACTION_TOPIC_SUFFIX;
  strcpy(_mqtt_fan_action_topic, handler->mqtt_clientId);
  strcat(_mqtt_fan_action_topic, fan_action_topic_suffix);

  const char *fan_status_topic_suffix = MQTT_FAN_STATUS_TOPIC_SUFFIX;
  strcpy(_mqtt_fan_status_topic, handler->mqtt_clientId);
  strcat(_mqtt_fan_status_topic, fan_status_topic_suffix);

  const char *fanspeed_action_topic_suffix = MQTT_FANSPEED_ACTION_TOPIC_SUFFIX;
  strcpy(_mqtt_fanspeed_action_topic, handler->mqtt_clientId);
  strcat(_mqtt_fanspeed_action_topic, fanspeed_action_topic_suffix);

  const char *fanspeed_status_topic_suffix = MQTT_FANSPEED_STATUS_TOPIC_SUFFIX;
  strcpy(_mqtt_fanspeed_status_topic, handler->mqtt_clientId);
  strcat(_mqtt_fanspeed_status_topic, fanspeed_status_topic_suffix);

  _fan_off_pin = fan_off_pin;
  _fan_low_pin = fan_low_pin;
  _fan_med_pin = fan_med_pin;
  _fan_high_pin = fan_high_pin;

#ifdef ACTIVE_HIGH_RELAY==true
  digitalWrite(_fan_off_pin, LOW);
  digitalWrite(_fan_low_pin, LOW);
  digitalWrite(_fan_med_pin, LOW);
  digitalWrite(_fan_high_pin, LOW);
  // Set output pins HIGH with an active-low relay
#else
  digitalWrite(_fan_off_pin, HIGH);
  digitalWrite(_fan_low_pin, HIGH);
  digitalWrite(_fan_med_pin, HIGH);
  digitalWrite(_fan_high_pin, HIGH);
#endif
  pinMode(_fan_off_pin, OUTPUT);
  pinMode(_fan_low_pin, OUTPUT);
  pinMode(_fan_med_pin, OUTPUT);
  pinMode(_fan_high_pin, OUTPUT);
  // Set output pins LOW with an active-high relay

  //Set fan to known off state
  _toggleRelay(fan_off_pin);
  _fan = STATE_OFF;
  _fanSpeed = FAN_MED;
  publish_fanSpeed_status();
  publish_fan_status();
}

void FanModule::onConnect() {
  Serial.print("Subscribing to ");
  Serial.print(_mqtt_fan_action_topic);
  Serial.print(" & ");
  Serial.print(_mqtt_fanspeed_action_topic);
  Serial.println("...");
  handler->client.subscribe(_mqtt_fan_action_topic);
  handler->client.subscribe(_mqtt_fanspeed_action_topic);

  // Publish the current status on connect/reconnect
  publish_fan_status();
  publish_fanSpeed_status();
}
bool FanModule::triggerAction(String topic, String payload) {

  static unsigned long fan_lastActionTime = 0;
  static unsigned long light_lastActionTime = 0;

  unsigned int currentTime = millis();

  if (topic == _mqtt_fan_action_topic && payload == FAN_ON_PAYLOAD && currentTime - fan_lastActionTime >= _actionTime) {
    fan_lastActionTime = currentTime;
    if (fan_onDelay) {
      _onFlag = true;
      _fan_onTime = millis() + fan_onDelay;
    }
    else {
      _fan = STATE_ON;
      if (_fanSpeed == FAN_HIGH) {
        _toggleRelay(_fan_high_pin);
      }
      else if (_fanSpeed == FAN_MED) {
        _toggleRelay(_fan_med_pin);
      }
      else if (_fanSpeed == FAN_LOW) {
        _toggleRelay(_fan_low_pin);
      }
      publish_fan_status();
      publish_fanSpeed_status();
    }
  }
  else if (topic == _mqtt_fan_action_topic && payload == FAN_OFF_PAYLOAD && currentTime - fan_lastActionTime >= _actionTime) {
    fan_lastActionTime = currentTime;
    _toggleRelay(_fan_off_pin);
    _fan = STATE_OFF;
    publish_fan_status();
    _onFlag = false;
  }
  else if (topic == _mqtt_fanspeed_action_topic && payload == FAN_HIGH_PAYLOAD && currentTime - fan_lastActionTime >= _actionTime) {
    fan_lastActionTime = currentTime;
    _fanSpeed = FAN_HIGH;
    if (_fan == STATE_ON) {
      _toggleRelay(_fan_high_pin);
    }
    publish_fanSpeed_status();
    publish_fan_status();
    _onFlag = false;
  }
  else if (topic == _mqtt_fanspeed_action_topic && payload == FAN_MED_PAYLOAD && currentTime - fan_lastActionTime >= _actionTime) {
    fan_lastActionTime = currentTime;
    _fanSpeed = FAN_MED;
    if (_fan == STATE_ON) {
      _toggleRelay(_fan_med_pin);
    }
    publish_fanSpeed_status();
    publish_fan_status();
    _onFlag = false;
  }
  else if (topic == _mqtt_fanspeed_action_topic && payload == FAN_LOW_PAYLOAD && currentTime - fan_lastActionTime >= _actionTime) {
    fan_lastActionTime = currentTime;
    _fanSpeed = FAN_LOW;
    if (_fan == STATE_ON) {
      _toggleRelay(_fan_low_pin);
    }
    publish_fanSpeed_status();
    publish_fan_status();
    _onFlag = false;
  }
  else {
    return false;
  }
  return true;
}

void FanModule::loop() {
  handle_FanOnDelay();
}
void FanModule::handle_FanOnDelay() {
  if (_onFlag && millis() >= _fan_onTime) {
    _onFlag = false;
    _fan = STATE_ON;
    if (_fanSpeed == FAN_HIGH) {
      _toggleRelay(_fan_high_pin);
    }
    else if (_fanSpeed == FAN_MED) {
      _toggleRelay(_fan_med_pin);
    }
    else if (_fanSpeed == FAN_LOW) {
      _toggleRelay(_fan_low_pin);
    }
    Serial.print("Delayed On... ");
    publish_fan_status();
    publish_fanSpeed_status();
  }
}
void FanModule::publish_fan_status() {
  Serial.print("Publishing Fan State: ");

  if (_fan == STATE_ON) {
    handler->client.publish(_mqtt_fan_status_topic, FAN_ON_PAYLOAD, true);
    Serial.println(FAN_ON_PAYLOAD);
  }
  else if (_fan == STATE_OFF) {
    handler->client.publish(_mqtt_fan_status_topic, FAN_OFF_PAYLOAD, true);
    Serial.println(FAN_OFF_PAYLOAD);
  }
  else {
    Serial.println("FAN STATE UNKNOWN");
  }
}
void FanModule::publish_fanSpeed_status() {
  Serial.print("Publishing Fan Speed: ");

  if (_fanSpeed == FAN_LOW) {
    handler->client.publish(_mqtt_fanspeed_status_topic, FAN_LOW_PAYLOAD, true);
    Serial.println(FAN_LOW_PAYLOAD);
  }
  else if (_fanSpeed == FAN_MED) {
    handler->client.publish(_mqtt_fanspeed_status_topic, FAN_MED_PAYLOAD, true);
    Serial.println(FAN_MED_PAYLOAD);
  }
  else if (_fanSpeed == FAN_HIGH) {
    handler->client.publish(_mqtt_fanspeed_status_topic, FAN_HIGH_PAYLOAD, true);
    Serial.println(FAN_HIGH_PAYLOAD);
  }
}

void FanModule::_toggleRelay(int pin) {
  //debug leds on while relay active
  digitalWrite(handler->led_pin, true); //red light off
  digitalWrite(handler->wifi_pin, false); //blue led on
#if(ACTIVE_HIGH_RELAY)
  digitalWrite(pin, HIGH);
  Serial.print("Toggled HIGH to Pin: ");
  Serial.println(pin);
  delay(RELAY_ACTIVE_TIME);
  digitalWrite(pin, LOW);
#else
  digitalWrite(pin, LOW);
  Serial.print("Toggled LOW to Pin: ");
  Serial.println(pin);
  delay(RELAY_ACTIVE_TIME);
  digitalWrite(pin, HIGH);
#endif
  //debug leds off
  digitalWrite(handler->wifi_pin, true); //blue led off
  digitalWrite(handler->led_pin, false);
}

