#include <Arduino.h>
#include <SPI.h>
#include "printf.h"
#include <nRF24L01.h>
#include "RF24.h"

// instantiate an object for the nRF24L01 transceiver
RF24 radio(4, 5); // CE pin, CSN pin

uint8_t address[][6] = {"1Node"};

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
float payload = 0.0;

void setup()
{

  Serial.begin(9600);
  delay(1000);

  Serial.println("Receiver Starting");

  // initialize the transceiver on the SPI bus
  if (!radio.begin())
  {
    while (1)
    {
      Serial.println(F("radio hardware is not responding!!"));
      delay(1000);
    } // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes
  radio.setRetries(15, 15);              // delay, count

  radio.openReadingPipe(1, address[0]); // using pipe 1
  radio.startListening();               // put radio in RX mode

  // For debugging info
  // printf_begin();             // needed only once for printing details
  radio.printPrettyDetails(); // (larger) function that prints human readable data

} // setup

void loop()
{
  // This device is a RX node

  uint8_t pipe;
  if (radio.available(&pipe))
  {                                         // is there a payload? get the pipe number that recieved it
    uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
    radio.read(&payload, bytes);            // fetch payload from FIFO
    Serial.print(F("Received "));
    Serial.print(bytes); // print the size of the payload
    Serial.print(F(" bytes on pipe "));
    Serial.print(pipe); // print the pipe number
    Serial.print(F(": "));
    Serial.println(payload); // print the payload's value
  }
} // loop