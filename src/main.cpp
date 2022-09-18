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
unsigned long dialingStartedAt = millis() - DIAL_DEBOUNCE_MILLIS;
char acSipIn[2048];
char acSipOut[2048];
Sip aSip(acSipOut, sizeof(acSipOut));

// RCSWITCH
RCSwitch mySwitch = RCSwitch();

bool saveConfig = false;

// Tasks
struct task
{
  unsigned long rate;
  unsigned long previous;
};

task taskA = {.rate = 1000, .previous = 0};

void publishMQTT(void)
{
  // MQTTclient.publish
  /*     MQTTclient.publish(topic.c_str(), char_Leistung_Zaehler4);
            topic = "Ferraris/";
            topic = topic + configManager.data.messure_place;
            topic = topic +"/Zähler4/UKWh";
            char char_meter_loop_counts4[5];
            dtostrf(configManager.data.meter_loops_count_4,4,0, char_meter_loop_counts4);
      MQTTclient.publish(topic.c_str(), char_meter_loop_counts4);
   */
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

  mySwitch.enableReceive(configManager.data.rcswitch_gpiopon);

  WiFi.hostname(configManager.data.wifi_hostname);
  WiFi.begin();

  if (configManager.data.mqtt_server && configManager.data.mqtt_port)
  {
    MQTTclient.setServer(configManager.data.mqtt_server, configManager.data.mqtt_port);
  }

  auto ipAddr = WiFi.localIP().toString().c_str();
  // TODO use random port?
  aSip.Init(configManager.data.sip_server, configManager.data.sip_port, ipAddr, 5060, configManager.data.sip_user, configManager.data.sip_password, configManager.data.sip_ringsecs);

}

void loop()
{
  // framework things
  WiFiManager.loop();
  updater.loop();
  configManager.loop();
  MQTTclient.loop();

  aSip.Processing(acSipIn, sizeof(acSipIn));

  // tasks
  if (taskA.previous == 0 || (millis() - taskA.previous > taskA.rate))
  {
    taskA.previous = millis();
    reconnect();
  }

  if (mySwitch.available())
  {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol());
    if (mySwitch.getReceivedValue() == configManager.data.rcswitch_value && (configManager.data.rcswitch_protocol < 0 || mySwitch.getReceivedProtocol() == configManager.data.rcswitch_protocol))
    {
      Serial.println("rccode matching");

      if (millis() >= dialingStartedAt + DIAL_DEBOUNCE_MILLIS)
      {
        Serial.print("dialing ");
        Serial.print(configManager.data.sip_numbertodial);
        Serial.print(" as ");
        Serial.println(configManager.data.sip_callername);
        aSip.Dial(configManager.data.sip_numbertodial, configManager.data.sip_callername);
        dialingStartedAt = millis();
      } else {
        Serial.println("Dialing already in progress");
      }
    }
    else
    {
      Serial.println("rccode does not match");
    }
    mySwitch.resetAvailable();
  }

  if (saveConfig)
  {
    saveConfig = false;
    configManager.save();
  }
}
