#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
#endif

#include "arduino_secrets.h"
char ssid[] = "iphone";    // network SSID (name)
char pass[] = "asdfghjkl";    // network password (use for WPA, or use as key for WEP)

const int trigPin = 2;
const int echoPin = 3;

//creating instance of different data types
float duration, distance;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

//declaring the broker, port and topic
const char broker[] = "broker.mqttdashboard.com";
int        port     = 1883;
const char topic[]  = "SIT210/wave";

// initialising to connect the arduino nano with the server
const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  // displaying the status when the wifi is connected to the arduino
  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    //setting up the ultra sonic sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration*.0343)/2;

    //displaying the distance on the serial monitor
    Serial.print("Distance: ");
    Serial.println(distance);
    //if the distance is less than 12 then a message will be sent on broker
    if(distance < 12){
    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print("Shravel, ");
    mqttClient.print("Wave is detected, ");
    mqttClient.print("Distance: ");
    mqttClient.print(distance);
    mqttClient.endMessage();
    delay(1000);
    }

    Serial.println();

    count++;
  }
}
