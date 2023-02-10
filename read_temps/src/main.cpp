#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define ONE_WIRE_BUS 14
#define NUM_SENSORS 2

// Init for radio
const byte address[5] = {'R', 'x', 'A', 'A', 'A'};
RF24 radio(4, 5); // CE_pin, CSN_pin Create a Radio

// Init for temp sensors
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int deviceCount = 0;
float Celcius = 0;
float Fahrenheit = 0;
float tempList[NUM_SENSORS] = {};

void setup()
{
  Serial.begin(9600);
  Serial.println("Transmitter Starting");

  // Setup for radio
  if (!radio.begin())
  {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
    {
      Serial.println("radio hardware is not responding!!");
      delay(1000);
    } // hold in infinite loop
  }
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN); // RF24_PA_MAX is default. _MIN for testing
  radio.setChannel(108);
  radio.setRetries(15, 15); // delay, count
  radio.openWritingPipe(address);

  // Setup for temp sensors
  sensors.begin();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  Serial.println("");
}

//====================

void send()
{

  delay(1000);
  bool rslt;
  Serial.println("Size of tempList: ");
  Serial.println(sizeof(tempList));
  rslt = radio.write(&tempList, sizeof(tempList));
  delay(500);

  Serial.print("Data Sent: ");
  for (int i = 0; i < deviceCount; i++)
  {
    Serial.print(tempList[i]);
    Serial.print(" ");
  }
  Serial.println("");
  if (rslt)
  {
    Serial.println("  Acknowledge received");
  }
  else
  {
    Serial.println("  Tx failed");
  }
}

//===============

void updateTemp()
{

  delay(1000);
  sensors.requestTemperatures();
  delay(1000);

  for (int i = 0; i < deviceCount; i++)
  {
    Celcius = sensors.getTempCByIndex(i);
    Fahrenheit = sensors.toFahrenheit(Celcius);
    tempList[i] = Fahrenheit;
  }
}

void loop()
{

  updateTemp();
  send();
  delay(15000);
}