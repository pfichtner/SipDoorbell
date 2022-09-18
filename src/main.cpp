#include <Arduino.h>
#include <PubSubClient.h>
#include "LittleFS.h"
#include "WiFiManager.h"
#include "webServer.h"
#include "updater.h"
#include "configManager.h"
#include "timeSync.h"
#include <RCSwitch.h>
#include <ArduinoSIP.h>

#include "rc_switch_output.h"

// MQTT
WiFiClient espClient;
int mqttReconnect;
PubSubClient MQTTclient(espClient);

// SIP
const long DIAL_DEBOUNCE_MILLIS = 5 * 1000;
boolean dialInProgress;
unsigned long dialingStartedAt;
char acSipIn[2048];
char acSipOut[2048];
Sip aSip(acSipOut, sizeof(acSipOut));

// RCSWITCH
RCSwitch mySwitch = RCSwitch();

// Tasks
struct task
{
  unsigned long rate;
  unsigned long previous;
};

task taskA = {.rate = 1000, .previous = 0};

void publishMQTT(boolean state)
{
  String topic = String(configManager.data.wifi_hostname) + "/sensor/state";
  MQTTclient.publish(topic.c_str(), state ? "ON" : "OFF");
}

void reconnect(void)
{
  if (++mqttReconnect > 60)
  {
    mqttReconnect = 0;
    if (!MQTTclient.connected())
    {
      String clientId = String(configManager.data.projectName);
      MQTTclient.connect(clientId.c_str(), configManager.data.mqtt_user, configManager.data.mqtt_password);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  LittleFS.begin();
  GUI.begin();
  configManager.begin();
  WiFiManager.begin(configManager.data.projectName);
  timeSync.begin();

  mySwitch.enableReceive(configManager.data.rcswitch_gpiopin);

  WiFi.hostname(configManager.data.wifi_hostname);
  WiFi.begin();

  if (configManager.data.mqtt_server[0] == '\0' || configManager.data.mqtt_port <= 0)
  {
    Serial.println("MQTT not configured");
  }
  else
  {
    MQTTclient.setServer(configManager.data.mqtt_server, configManager.data.mqtt_port);
    Serial.print("Configuring MQTT Broker ");
    Serial.print(configManager.data.mqtt_server);
    Serial.print(":");
    Serial.print(configManager.data.mqtt_port);
  }

  auto ipAddr = WiFi.localIP().toString().c_str();
  // TODO use random port?
  auto port = 5060;

  Serial.print("Configuring SIP server ");
  Serial.print(configManager.data.sip_server);
  Serial.print(":");
  Serial.print(configManager.data.sip_port);
  Serial.print(" using ");
  Serial.print(configManager.data.sip_user);
  Serial.print("@");
  Serial.print(configManager.data.sip_password);
  Serial.print(" (local ");
  Serial.print(ipAddr);
  Serial.print(":");
  Serial.print(port);
  Serial.println(")");

  aSip.Init(configManager.data.sip_server, configManager.data.sip_port, ipAddr, port, configManager.data.sip_user, configManager.data.sip_password, configManager.data.sip_ringsecs);
}

void switchPin(boolean state)
{
  if (configManager.data.switch_gpiopin > 0)
  {
    digitalWrite(configManager.data.switch_gpiopin, state ? 1 : 0);
  }
}

void setDialInProgress(boolean dialInProgress_)
{
  if (dialInProgress != dialInProgress_)
  {
    dialInProgress = dialInProgress_;
    switchPin(dialInProgress);
    publishMQTT(dialInProgress);
  }
}

void dial(void)
{
  if (dialInProgress)
  {
    Serial.println("Dialing already in progress");
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

void SIPloop(void)
{
  aSip.Processing(acSipIn, sizeof(acSipIn));
}

void RCSwitchLoop(void)
{
  if (mySwitch.available())
  {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
    if (mySwitch.getReceivedValue() == configManager.data.rcswitch_value && (configManager.data.rcswitch_protocol < 0 || mySwitch.getReceivedProtocol() == configManager.data.rcswitch_protocol))
    {
      Serial.println("rccode matching");
      dial();
    }
    else
    {
      Serial.println("rccode does not match");
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
  MQTTclient.loop();

  if (dialInProgress && millis() >= dialingStartedAt + DIAL_DEBOUNCE_MILLIS)
  {
    setDialInProgress(false);
  }

  SIPloop();
  RCSwitchLoop();

  // tasks
  if (taskA.previous == 0 || (millis() - taskA.previous > taskA.rate))
  {
    taskA.previous = millis();
    reconnect();
  }
}
