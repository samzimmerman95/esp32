#include <Arduino.h>
#include <SPI.h>
// #include "printf.h"
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

  Serial.println("Transmitter Starting");

  // initialize the transceiver on the SPI bus
  if (!radio.begin())
  {
    while (1)
    {
      Serial.println("radio hardware is not responding!!");
      delay(1000);
    } // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload)); // float datatype occupies 4 bytes
  radio.setRetries(15, 15);              // delay, count

  radio.openWritingPipe(address[0]); // always uses pipe 0

  // For debugging info
  // printf_begin();             // needed only once for printing details
  radio.printPrettyDetails(); // (larger) function that prints human readable data

} // setup

void loop()
{
  // This device is a TX node

  unsigned long start_timer = micros();               // start the timer
  bool report = radio.write(&payload, sizeof(float)); // transmit & save the report
  unsigned long end_timer = micros();                 // end the timer
  delay(500);

  if (report)
  {
    Serial.print("Transmission successful! "); // payload was delivered
    Serial.print("Time to transmit = ");
    Serial.print(end_timer - start_timer); // print the timer result
    Serial.print(" us. Sent: ");
    Serial.println(payload); // print payload sent
    payload += 0.01;         // increment float payload
  }
  else
  {
    Serial.println("Transmission failed or timed out"); // payload was not delivered
  }

  delay(5000); // slow transmissions down by 1 second

} // loop