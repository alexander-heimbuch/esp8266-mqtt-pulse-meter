#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

extern "C" {
#include "gpio.h"
#include "user_interface.h"
}

#define PIN D1                // ESP8266 pin
#define UPDATE_INTERVAL 180   // Interval in Seconds
#define IMPULSE 0.01          // Update Intervall
#define UNIT "qm"             // Calculate Unit

int pulseCounter;
Ticker Timer;

const char *MQTT_CLIENT = "my-client";
const char *MQTT_TOPIC = "my/mqtt/topic";
const char *Ssid =  "";             // cannot be longer than 32 characters! there also might be an issue with connection if SSID is less than 8 chars
const char *Pass =  "";             // Wlan Password
char MqttServer[] = "";             // address of your MQTT Server
unsigned int MqttPort = 1883;      // MQTT port number. default is 1883

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("mqtt callback invoked. not implemented.");
}

WiFiClient WifiClient;
PubSubClient MqttClient(MqttServer, MqttPort, mqttCallback, WifiClient);

// Example Payload:
// {
//   "value": 0.01,
//   "unit": "qm",
//   "interval": 180
// }
void publishMqtt() {
  String payload = "{";
  payload += "\"value\":";
  payload += (float)pulseCounter * IMPULSE;
  payload += ",\"unit\":";
  payload += UNIT;
  payload += ",\"interval\":";
  payload += UPDATE_INTERVAL;
  payload += "}";

  MqttClient.publish(MQTT_TOPIC, (char*) payload.c_str());
}
 
IRAM_ATTR void pulse() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(1);
  digitalWrite(LED_BUILTIN, HIGH);
  pulseCounter++;
}

void update()
{
  publishMqtt();
  pulseCounter = 0;
}
 
void setup()
{
  Serial.begin(74880);
  delay(10);
  WiFi.mode(WIFI_STA);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Timer.attach(UPDATE_INTERVAL, update);
  attachInterrupt(digitalPinToInterrupt(PIN), pulse, FALLING);
}
 
void loop()
{
 
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(Ssid);
    Serial.println("...");
    WiFi.begin(Ssid, Pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      WiFi.printDiag(Serial);
      return;
    }
    Serial.println("WiFi connected");
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!MqttClient.connected()) {
      yield();
      if (MqttClient.connect(MQTT_CLIENT)) {
        yield();
        Serial.println("MQTT connected");
      }
    }

    if (MqttClient.connected())
      MqttClient.loop();
  }

  yield();
}
