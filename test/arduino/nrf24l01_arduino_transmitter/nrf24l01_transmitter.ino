
#include <SPI.h>
#include "RF24.h"

RF24 radio(9,8); // CE,CSN

void setup() {
  Serial.begin(115200);
  Serial.println(F("Transmitter Started"));
  
  radio.begin();
  radio.setAutoAck(1);

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(4);
                      
  byte tx_pipe_address[5] = { 0x01,0x02,0x03,0x04,0x05 };
  byte rx_pipe0_address[5] = { 0x0A,0x0B,0x0C,0x0D,0x0E };

  // Open a writing and reading pipe
  radio.openWritingPipe(tx_pipe_address);
  radio.openReadingPipe(0,rx_pipe0_address);
 }

void loop() {
 
    radio.stopListening();                                    // First, stop listening so we can talk.
    
    Serial.println(F("Now sending"));

    unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
     if (!radio.write( &start_time, sizeof(unsigned long) )){
       Serial.println(F("failed"));
     }
     
    // Try again 1s later
    delay(1000);

} // Loop
