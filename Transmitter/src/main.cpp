#include <Arduino.h>
#include <SPI.h>
// #include "printf.h"
#include <nRF24L01.h>
#include "RF24.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 14
#define NUM_SENSORS 5
#define PWR_CONTROLLER_DONE 22

// instantiate an object for the nRF24L01 transceiver
RF24 radio(4, 5); // CE pin, CSN pin
uint8_t address[][6] = {"1Node"};

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
  delay(1000);

  Serial.println("Transmitter Starting");
  pinMode(PWR_CONTROLLER_DONE, OUTPUT);
  if (!radio.begin())
  {
    while (1)
    {
      Serial.println("radio hardware is not responding!!");
      delay(1000);
    }
  }

  radio.setPALevel(RF24_PA_MAX); // RF24_PA_MAX is default.
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(115);

  // save on transmission time by setting the radio to only transmit the size we need
  radio.setPayloadSize(sizeof(tempList));
  radio.setRetries(15, 15); // delay, count

  radio.openWritingPipe(address[0]); // always uses pipe 0

  // For debugging info
  radio.printPrettyDetails(); // (larger) function that prints human readable data

  // Setup for temp sensors
  sensors.begin();
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount);
  Serial.println(" devices.");
  Serial.println("");

} // setup

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
  // This device is a TX node

  unsigned long start_timer = micros();                   // start the timer
  bool report = radio.write(&tempList, sizeof(tempList)); // transmit & save the report
  unsigned long end_timer = micros();                     // end the timer
  delay(500);

  if (report)
  {
    Serial.print("Transmission successful! "); // payload was delivered
    Serial.print("Time to transmit = ");
    Serial.print(end_timer - start_timer); // print the timer result
    Serial.print(" us. Sent: ");
    for (int i = 0; i < deviceCount; i++)
    {
      Serial.print(tempList[i]);
      Serial.print(" ");
    }
    Serial.println("");
  }
  else
  {
    Serial.println("Transmission failed or timed out"); // payload was not delivered
  }

  delay(5000);
  digitalWrite(PWR_CONTROLLER_DONE, HIGH);
  // delay(5000); // slow transmissions down

} // loop
