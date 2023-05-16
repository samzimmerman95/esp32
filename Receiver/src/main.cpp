#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include "RF24.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <iostream>
#include <secrets.h> //Header file with secrets define. Not tracked in git

#define NUM_SENSORS 5
#define LED 2

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJsonArray send_array;
unsigned int epochTime;
int count = 0;

// instantiate an object for the nRF24L01 transceiver
RF24 radio(4, 5); // CE pin, CSN pin
uint8_t address[][6] = {"1Node"};
float payloadList[NUM_SENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0};
const float zero = 0.0;

// Replace with your network credentials (STATION)
const char *ssid = SECRET_WIFI_NETWORK;
const char *password = SECRET_WIFI_PASSWORD;

void initWiFi()
{
  // delete old config
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print(F("Connecting to WiFi .."));
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
    if (count > 25)
    {
      count = 0;
      ESP.restart();
    }
    count = count + 1;
  }
  Serial.println(F(""));
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("RRSI: "));
  Serial.println(WiFi.RSSI());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  // initWiFi();
  Serial.println(F("WiFi disconnected. Restarting"));
  ESP.restart();
}

void init_firebase()
{
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
}

const char *ntpServer = "pool.ntp.org";
// Function that gets current epoch time
unsigned long getTime()
{
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return (0);
  }
  time(&now);
  return now;
}

void blink()
{
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
}

void setup()
{
  Serial.begin(9600);
  delay(1000);
  initWiFi();
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  pinMode(LED, OUTPUT);
  configTime(0, 0, ntpServer);
  init_firebase();

  // Serial.println(F("Receiver Starting"));

  // initialize the transceiver on the SPI bus
  if (!radio.begin())
  {
    while (1)
    {
      Serial.println(F("radio hardware is not responding!!"));
      delay(1000);
      if (count > 25)
      {
        count = 0;
        ESP.restart();
      }
      count = count + 1;
    } // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_MAX); // RF24_PA_MAX is default.
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(115);
  radio.setPayloadSize(sizeof(payloadList));
  radio.setRetries(15, 15);             // delay, count
  radio.openReadingPipe(1, address[0]); // using pipe 1
  radio.startListening();               // put radio in RX mode

  // For debugging info
  radio.printPrettyDetails();

} // setup

void loop()
{
  // This device is a RX node
  uint8_t pipe;
  if (radio.available(&pipe))
  {                                         // is there a payload? get the pipe number that recieved it
    uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
    radio.read(&payloadList, bytes);        // fetch payload from FIFO
    // Serial.print(F("Received "));
    // Serial.print(bytes); // print the size of the payload
    // Serial.print(F(" bytes on pipe "));
    // Serial.print(pipe); // print the pipe number
    // Serial.print(F(": "));
    for (int i = 0; i < NUM_SENSORS; i++)
    {
      Serial.print(payloadList[i]);
      Serial.print(" ");
      if (payloadList[i] > 10)
      {
        char payload_str[10];
        sprintf(payload_str, "%.2f", payloadList[i]);
        sscanf(payload_str, "%f", &payloadList[i]);
        send_array.add(payloadList[i]);
      }
    }
    Serial.println("");
  }
  if (Firebase.ready() && send_array.size() > 0)
  {
    Serial.println(F("Firebase Ready and data found"));
    epochTime = getTime();
    char path[25] = "";
    sprintf(path, "/lake23/%i000", epochTime);
    // Serial.println(path);
    Serial_Printf("Set temp ... %s\n", Firebase.RTDB.setArray(&fbdo, path, &send_array) ? "ok" : fbdo.errorReason().c_str());
    send_array.clear();
    blink();
  }
} // loop
