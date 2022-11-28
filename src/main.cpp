// ******************************************************************
// MQTT Read Lab
// COMP-10184
// @author Justin Borzi
// @id 000798465
//
// I, Justin Borzi, 000798465, certify that this material is my original work.
// No other person's work has been used without due acknowledgement and I have not made my work available to anyone else.

#define MQTT_MAX_PACKET_SIZE 4096

// Include the required libraries for this to run.
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>

// Add the WIFI Secrets file
#include "wifi_secrets.h"

// Wifi login details
char ssid[] = SECRET_SSID; // Change to your network SSID (name).
char pass[] = SECRET_PASS; // Change to your network password.

// interface to ThingSpeak MQTT interface
const char *mqttServer = "test.mosquitto.org";
const uint16_t mqttPort = 1883;

// Initiate the LED state
bool ledState = false;

// Pin that the  DS18B20 is connected to
const int oneWireBus = D3;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature DS18B20(&oneWire);

// Initilize the Wifi client and the channel client
WiFiClient espClient;
PubSubClient client(espClient);


void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("", "", ""))
    {
      Serial.println("connected");

      // Initialize a float value.
      float fTemp;

      // Request DS18B20 (devices) for the current temperature(s)
      DS18B20.requestTemperatures();

      // fetch the temperature.  We only have 1 sensor, so the index is 0.
      fTemp = DS18B20.getTempCByIndex(0);

      // map the temperature to a byte array.
      char array[10];
      sprintf(array, "%f", fTemp);

      // Once connected, publish an announcement...
      client.publish("MohawkCollege/AC/000798465/CurrentTemperature", array);

      // Get the time since started
      unsigned long allSeconds = millis() / 1000;
      int secsRemaining = allSeconds % 3600;
      int hours = allSeconds / 3600;
      int minutes = secsRemaining / 60;
      int seconds = secsRemaining % 60;

      // set the buffer to the time values in a formatted string
      char buf[15];
      sprintf(buf, "%02d:%02d:%02d", hours, minutes, seconds);
      client.publish("MohawkCollege/AC/000798465/TimeSinceStarted", buf);

      // ... and resubscribe
      client.subscribe("MohawkCollege/AC/000777218/CurrentTemperature");
      client.subscribe("MohawkCollege/AC/000777218/TimeSinceStarted");
      client.subscribe("MohawkCollege/AC/000798465/State");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Handle the subsciptons callback
void callback(char *topic, byte *payload, unsigned int length)
{

  // Print the message recieved
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Convert topic to a string
  String topicStr(topic);

  // check if channel is to power on and off device (virtually)
  if (topicStr.equals("MohawkCollege/AC/000798465/State"))
  {
    Serial.println(String((char)payload[0]) == "1");
    ledState = String((char)payload[0]) == "1";
  }
}

// setup the wifi connection sequence
void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// ****************************************************************************
void setup()
{

  // configure pin D4 as a digital output - this is the LED
  pinMode(D4, OUTPUT);
  // start debug console
  Serial.begin(115200);
  Serial.print("\n\nJustin Borzi, 000798465");
  setup_wifi();
  // set the server to use and the function to use for callbacks
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

// ****************************************************************************
void loop()
{
  // set the state of the "power"
  digitalWrite(D4, ledState ? LOW : HIGH);

  // Listen to MQTT Channels
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}