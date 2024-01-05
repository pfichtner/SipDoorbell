#include <Arduino.h>
#include <PubSubClient.h>
#include "LittleFS.h"
#include "WiFiManager.h"
#include "webServer.h"
#include "updater.h"
#include "configManager.h"
#include "dashboard.h"
#include <RCSwitch.h>
#include <ArduinoSIP.h>

#include "rc_switch_output.h"

#define DTMF_HANDLING

// WIFI
WiFiClient espClient;
String localIp;

// MQTT
PubSubClient mqttClient(espClient);

// SIP
char acSipIn[2048];
char acSipOut[2048];
Sip aSip(acSipOut, sizeof(acSipOut));
unsigned long dialingStartedAt;

// RCSWITCH
RCSwitch mySwitch = RCSwitch();

// gpio input can be configured pullup and pulldown, so HIGH or LOW could signal a button press
int pin_pressed_is = HIGH;

// Tasks
struct task
{
  unsigned long rate;
  unsigned long previous;
};

task taskA = {.rate = 10 * 1000, .previous = 0};

void mqttPublish(boolean state)
{
  String stateTopic = String(configManager.data.wifi_hostname) + "/sensor/state";
  mqttClient.publish(stateTopic.c_str(), state ? "ON" : "OFF");
}

void mqttReconnect(void)
{
  dash.data.MQTT_Connected = mqttClient.connected();
  if (!dash.data.MQTT_Connected)
  {
    String clientId = String(configManager.data.projectName);
    mqttClient.connect(clientId.c_str(), configManager.data.mqtt_user, configManager.data.mqtt_password);
    Serial.print(F("Connected to MQTT Broker, clientId "));
    Serial.println(clientId);
  }
}

void sipBegin(void)
{
  // TODO use random port?
  // int localPort = random(1 * 1024, 64 * 1024);
  int localPort = 5060;

  Serial.print(F("Configuring SIP connection from "));
  Serial.print(localIp);
  Serial.print(":");
  Serial.print(localPort);
  Serial.print(F(" to "));
  Serial.print(configManager.data.sip_server);
  Serial.print(":");
  Serial.println(configManager.data.sip_port);

  aSip.Init(configManager.data.sip_server, configManager.data.sip_port, localIp.c_str(), localPort, configManager.data.sip_user, configManager.data.sip_password, configManager.data.sip_ringsecs);
}

void mqttBegin(void)
{
  if (strlen(configManager.data.mqtt_server) == 0 || configManager.data.mqtt_port <= 0)
  {
    Serial.println("MQTT not configured");
  }
  else
  {
    mqttClient.setServer(configManager.data.mqtt_server, configManager.data.mqtt_port);
    Serial.print(F("Configuring MQTT Broker "));
    Serial.print(configManager.data.mqtt_server);
    Serial.print(":");
    Serial.println(configManager.data.mqtt_port);
  }
}

void inputPinBegin(void)
{
  if (configManager.data.button_gpiopin > 0)
  {
    Serial.print(configManager.data.button_gpiopin);
    Serial.println(F(" configured for input button"));

    if (strcmp("PULLUP", configManager.data.button_gpiopin_mode))
    {
      pinMode(configManager.data.button_gpiopin, INPUT);
      pin_pressed_is = LOW;
    }
    else if (strcmp("PULLUP (internal)", configManager.data.button_gpiopin_mode))
    {
      pinMode(configManager.data.button_gpiopin, INPUT_PULLUP);
      pin_pressed_is = LOW;
    }
    else 
    {
      // PULLDOWN
      pinMode(configManager.data.button_gpiopin, INPUT);
      pin_pressed_is = HIGH;
    }
  }
  else
  {
    Serial.println("no input button configured");
  }
}

void outputPinBegin(void)
{
  if (configManager.data.switch_gpiopin > 0) 
  {
    pinMode(configManager.data.switch_gpiopin, OUTPUT);
    digitalWrite(configManager.data.switch_gpiopin, LOW);
  }
}

void rcSwitchBegin(void)
{
  mySwitch.enableReceive(configManager.data.rcswitch_gpiopin);
}

void configDependendBegins(void)
{
  mqttBegin();
  sipBegin();

  inputPinBegin();
  outputPinBegin();
  rcSwitchBegin();
}

void setup()
{
  Serial.begin(115200);

  LittleFS.begin();
  GUI.begin();
  configManager.begin();
  configManager.setConfigSaveCallback(configDependendBegins);
  WiFiManager.begin(configManager.data.projectName);
  dash.begin(500);
  WiFi.hostname(configManager.data.wifi_hostname);
  WiFi.begin();

  localIp = WiFi.localIP().toString();
  Serial.println(F("Connected"));

  configDependendBegins();
}

void switchPin(boolean state)
{
  if (configManager.data.switch_gpiopin > 0)
  {
    digitalWrite(configManager.data.switch_gpiopin, state ? HIGH : LOW);
    Serial.print(F("switched pin "));
    Serial.print(configManager.data.button_gpiopin);
    Serial.println(": ");
    Serial.println(state ? "HIGH" : "LOW");
  }
}

void setDialInProgress(boolean dialInProgress)
{
  if (dash.data.dialInProgress != dialInProgress)
  {
    dash.data.dialInProgress = dialInProgress;
    switchPin(dialInProgress);
    mqttPublish(dialInProgress);
  }
}

void dial(void)
{
  if (dash.data.dialInProgress)
  {
    Serial.println(F("Dialing already in progress"));
    return;
  }
  setDialInProgress(true);
  dialingStartedAt = millis();
  Serial.print("dialing ");
  Serial.print(configManager.data.sip_numbertodial);
  Serial.print(" as ");
  Serial.println(configManager.data.sip_callername);
  aSip.Dial(configManager.data.sip_numbertodial, configManager.data.sip_callername);
}

void buttonLoop(void)
{
  if (digitalRead(configManager.data.button_gpiopin) == pin_pressed_is)
  {
    Serial.print(configManager.data.button_gpiopin);
    Serial.println(F(" button press detected"));
    dial();
  }
}

void sipLoop(void)
{
  aSip.Processing(acSipIn, sizeof(acSipIn));
#ifdef DTMF_HANDLING
  // preparation for possible opener
  char iSignal = aSip.GetSignal();
  if (iSignal)
  {
    Serial.print("Signal received: ");
    Serial.println(iSignal);
  }
#endif
}

void rcSwitchLoop(void)
{
  if (mySwitch.available())
  {
    dash.data.rcswitch_value = mySwitch.getReceivedValue();
    dash.data.rcswitch_protocol = mySwitch.getReceivedProtocol();
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
    if (mySwitch.getReceivedValue() == configManager.data.rcswitch_value && (configManager.data.rcswitch_protocol < 0 || mySwitch.getReceivedProtocol() == configManager.data.rcswitch_protocol))
    {
      Serial.println(F("rc-code matching"));
      dial();
    }
    else
    {
      Serial.println(F("rc-code does not match"));
    }
    mySwitch.resetAvailable();
  }
}

void loop()
{
  // framework things
  WiFiManager.loop();
  updater.loop();
  configManager.loop();
  dash.loop();
  mqttClient.loop();

  unsigned long now = millis();

  if (dash.data.dialInProgress && now >= dialingStartedAt + configManager.data.sip_ringsecs * 1000)
  {
    setDialInProgress(false);
  }

  buttonLoop();
  sipLoop();
  rcSwitchLoop();

  // tasks
  if (taskA.previous == 0 || (now - taskA.previous > taskA.rate))
  {
    taskA.previous = now;
    dash.data.WiFi_RSSI = WiFi.RSSI();
    mqttReconnect();
  }
}
