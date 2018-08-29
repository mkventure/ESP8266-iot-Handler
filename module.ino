enum FanSpeed {
  FAN_HIGH,
  FAN_MED,
  FAN_LOW
};

enum Ternary {
  STATE_UNKNOWN,
  STATE_ON,
  STATE_OFF
};

const int debounceTime = DEBOUNCE_TIME;
const int actionTime = ACTION_TIME;
const int relayActiveTime = RELAY_ACTIVE_TIME;

const char* mqtt_fan_action_topic = MQTT_FAN_ACTION_TOPIC;
const char* mqtt_fan_status_topic = MQTT_FAN_STATUS_TOPIC;
const char* mqtt_fanspeed_action_topic = MQTT_FANSPEED_ACTION_TOPIC;
const char* mqtt_fanspeed_status_topic = MQTT_FANSPEED_STATUS_TOPIC;
enum FanSpeed fanSpeed = FAN_MED;
enum Ternary fan = STATE_UNKNOWN;

const char* mqtt_light_action_topic = MQTT_LIGHT_ACTION_TOPIC;
const char* mqtt_light_status_topic = MQTT_LIGHT_STATUS_TOPIC;
enum Ternary light = STATE_ON;

const char* mqtt_command_action_topic = MQTT_COMMAND_ACTION_TOPIC;
const char* invert_light = LIGHT_INVERT_PAYLOAD;

#ifdef FAN_ON_DELAY
bool onFlag = false;
const unsigned long fan_onDelay = FAN_ON_DELAY;
unsigned long fan_onTime;
#endif


void setupModule() {
  Serial.println("Starting FanControl...");
  pinMode(FAN_OFF_PIN, OUTPUT);
  pinMode(FAN_LOW_PIN, OUTPUT);
  pinMode(FAN_MED_PIN, OUTPUT);
  pinMode(FAN_HIGH_PIN, OUTPUT);
  pinMode(LIGHT_ON_PIN, OUTPUT);
  pinMode(LIGHT_OFF_PIN, OUTPUT);
  // Set output pins LOW with an active-high relay
#ifdef ACTIVE_HIGH_RELAY==true
  digitalWrite(FAN_OFF_PIN, LOW);
  digitalWrite(FAN_LOW_PIN, LOW);
  digitalWrite(FAN_MED_PIN, LOW);
  digitalWrite(FAN_HIGH_PIN, LOW);
  digitalWrite(LIGHT_ON_PIN, LOW);
  digitalWrite(LIGHT_OFF_PIN, LOW);
  // Set output pins HIGH with an active-low relay
#else
  digitalWrite(FAN_OFF_PIN, HIGH);
  digitalWrite(FAN_LOW_PIN, HIGH);
  digitalWrite(FAN_MED_PIN, HIGH);
  digitalWrite(FAN_HIGH_PIN, HIGH);
  digitalWrite(LIGHT_ON_PIN, HIGH);
  digitalWrite(LIGHT_OFF_PIN, HIGH);
#endif

  //Set fan to known off state
  toggleRelay(FAN_OFF_PIN);
  fan = STATE_OFF;

  light = STATE_OFF;
}

void loopModule() {
#ifdef FAN_ON_DELAY
handle_FanOnDelay();
#endif
}

void onConnect() {
  // Subscribe to the action topics to listen for action messages
  Serial.print("Subscribing to ");
  Serial.print(mqtt_fan_action_topic);
  Serial.print(" & ");
  Serial.print(mqtt_fanspeed_action_topic);
  Serial.println("...");
  iotHandler.client.subscribe(mqtt_fan_action_topic);
  iotHandler.client.subscribe(mqtt_fanspeed_action_topic);

  Serial.print("Subscribing to ");
  Serial.print(mqtt_light_action_topic);
  Serial.println("...");
  iotHandler.client.subscribe(mqtt_light_action_topic);

  Serial.print("Subscribing to ");
  Serial.print(mqtt_command_action_topic);
  Serial.println("...");
  iotHandler.client.subscribe(mqtt_command_action_topic);  

  // Publish the current status on connect/reconnect
  publish_fan_status();
  publish_fanSpeed_status();
  publish_light_status();
}

#ifdef FAN_ON_DELAY
void handle_FanOnDelay() {
  if (onFlag && millis() >= fan_onTime) {
    onFlag = false;    
    fan = STATE_ON;
    if (fanSpeed == FAN_HIGH) {
      toggleRelay(FAN_HIGH_PIN);
    }
    else if (fanSpeed == FAN_MED) {
      toggleRelay(FAN_MED_PIN);
    }
    else if (fanSpeed == FAN_LOW) {
      toggleRelay(FAN_LOW_PIN);
    }
    Serial.print("Delayed On... ");
    publish_fan_status();
    publish_fanSpeed_status();
  }
}
#endif

void publish_light_status() {
  Serial.print("Publishing Light State: ");

  if (light == STATE_ON) {
    iotHandler.client.publish(mqtt_light_status_topic, LIGHT_ON_PAYLOAD, true);
    Serial.println(LIGHT_ON_PAYLOAD);
  }
  else if (light == STATE_OFF) {
    iotHandler.client.publish(mqtt_light_status_topic, LIGHT_OFF_PAYLOAD, true);
    Serial.println(LIGHT_OFF_PAYLOAD);
  }
}

void publish_fan_status() {
  Serial.print("Publishing Fan State: ");

  if (fan == STATE_ON) {
    iotHandler.client.publish(mqtt_fan_status_topic, FAN_ON_PAYLOAD, true);
    Serial.println(FAN_ON_PAYLOAD);
  }
  else if (fan == STATE_OFF) {
    iotHandler.client.publish(mqtt_fan_status_topic, FAN_OFF_PAYLOAD, true);
    Serial.println(FAN_OFF_PAYLOAD);
  }
  else {    
    Serial.println("FAN STATE UNKNOWN");
  }
}

void publish_fanSpeed_status() {
  Serial.print("Publishing Fan Speed: ");

  if (fanSpeed == FAN_LOW) {
    iotHandler.client.publish(mqtt_fanspeed_status_topic, FAN_LOW_PAYLOAD, true);
    Serial.println(FAN_LOW_PAYLOAD);
  }
  else if (fanSpeed == FAN_MED) {
    iotHandler.client.publish(mqtt_fanspeed_status_topic, FAN_MED_PAYLOAD, true);
    Serial.println(FAN_MED_PAYLOAD);
  }
  else if (fanSpeed == FAN_HIGH) {
    iotHandler.client.publish(mqtt_fanspeed_status_topic, FAN_HIGH_PAYLOAD, true);
    Serial.println(FAN_HIGH_PAYLOAD);
  }
}

void toggleRelay(int pin) {
  //debug leds on while relay active
  digitalWrite(LED_PIN, true); //red light off
  digitalWrite(WIFI_PIN, false); //blue led on
#if(ACTIVE_HIGH_RELAY)
  digitalWrite(pin, HIGH);
  Serial.print("Toggled HIGH to Pin: ");
  Serial.println(pin);
  delay(relayActiveTime);
  digitalWrite(pin, LOW);
#else
  digitalWrite(pin, LOW);
  Serial.print("Toggled LOW to Pin: ");
  Serial.println(pin);
  delay(relayActiveTime);
  digitalWrite(pin, HIGH);
#endif
  //debug leds off
  digitalWrite(WIFI_PIN, true); //blue led off
  digitalWrite(LED_PIN, false);
}


void triggerAction(String topic, String payload) {
  static unsigned long fan_lastActionTime = 0;
  static unsigned long light_lastActionTime = 0;

  unsigned int currentTime = millis();

  if (topic == mqtt_fan_action_topic && payload == FAN_ON_PAYLOAD && currentTime - fan_lastActionTime >= actionTime) {
    fan_lastActionTime = currentTime;
    #ifdef FAN_ON_DELAY
    onFlag = true;
    fan_onTime = millis() + fan_onDelay;
    #else
    fan = STATE_ON;
    if (fanSpeed == FAN_HIGH) {
      toggleRelay(FAN_HIGH_PIN);
    }
    else if (fanSpeed == FAN_MED) {
      toggleRelay(FAN_MED_PIN);
    }
    else if (fanSpeed == FAN_LOW) {
      toggleRelay(FAN_LOW_PIN);
    }
    publish_fan_status();
    publish_fanSpeed_status();
    #endif
  }
  else if (topic == mqtt_fan_action_topic && payload == FAN_OFF_PAYLOAD && currentTime - fan_lastActionTime >= actionTime) {
    fan_lastActionTime = currentTime;
    toggleRelay(FAN_OFF_PIN);
    fan = STATE_OFF;
    publish_fan_status();
    onFlag = false;
  }
  else if (topic == mqtt_fanspeed_action_topic && payload == FAN_HIGH_PAYLOAD && currentTime - fan_lastActionTime >= actionTime) {
    fan_lastActionTime = currentTime;
    fanSpeed = FAN_HIGH;
    if (fan == STATE_ON) {
      toggleRelay(FAN_HIGH_PIN);
    }
    publish_fanSpeed_status();
    publish_fan_status();
    onFlag = false;
  }  
  else if (topic == mqtt_fanspeed_action_topic && payload == FAN_MED_PAYLOAD && currentTime - fan_lastActionTime >= actionTime) {
    fan_lastActionTime = currentTime;
    fanSpeed = FAN_MED;
    if (fan == STATE_ON) {
      toggleRelay(FAN_MED_PIN);
    }
    publish_fanSpeed_status();
    publish_fan_status();
    onFlag = false;
  }
  else if (topic == mqtt_fanspeed_action_topic && payload == FAN_LOW_PAYLOAD && currentTime - fan_lastActionTime >= actionTime) {
    fan_lastActionTime = currentTime;
    fanSpeed = FAN_LOW;
    if (fan == STATE_ON) {
      toggleRelay(FAN_LOW_PIN);
    }
    publish_fanSpeed_status();
    publish_fan_status();
    onFlag = false;
  }
  else if (topic == mqtt_light_action_topic && payload == LIGHT_ON_PAYLOAD && currentTime - light_lastActionTime >= actionTime && light != STATE_ON) {
    light_lastActionTime = currentTime;
    toggleRelay(LIGHT_ON_PIN);
    light = STATE_ON;
    publish_light_status();
  }
  else if (topic == mqtt_light_action_topic && payload == LIGHT_OFF_PAYLOAD && currentTime - light_lastActionTime >= actionTime && light != STATE_OFF) {
    light_lastActionTime = currentTime;
    toggleRelay(LIGHT_OFF_PIN);
    light = STATE_OFF;
    publish_light_status();
  }
  else if (topic == mqtt_command_action_topic && payload == invert_light) {
    if(light == STATE_OFF) {
      light = STATE_ON;
    }
    else {
      light = STATE_OFF;      
    }
    publish_light_status();
  }
  else {
    Serial.println("Unrecognized action payload... taking no action!");
  }
}
