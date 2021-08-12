
#include <SPI.h>
#include "RF24.h"

RF24 radio(9,8); // CE,CSN
uint8_t buffer[32] = {0};

void setup() {
  Serial.begin(115200);
  Serial.println(F("Receiver started"));
  
  radio.begin();
  radio.setAutoAck(1);

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(32);

  byte tx_pipe_address[5] = { 0x0A,0x0B,0x0C,0x0D,0x0E };
  byte rx_pipe1_address[5] = { 0x01,0x02,0x03,0x04,0x05 };
  
  radio.openWritingPipe(tx_pipe_address);
  radio.openReadingPipe(1,rx_pipe1_address);

  // Start the radio listening for data
  radio.startListening();
}

void loop() {
  
  unsigned long got_time;
    
  if( radio.available()){
                                                                    // Variable for the received timestamp
    while (radio.available()) {                                   // While there is data ready
      radio.read( &buffer[0], sizeof(buffer) );             // Get the payload
    }

    Serial.println("Received packet:");
    for(uint8_t i = 0; i < sizeof(buffer); i++){
      Serial.print("0x");
      char buf[3];
      sprintf(buf, "%02x", buffer[i]); 
      Serial.print(buf);
      if(i != 0 && (i+1) % 8 == 0){
        Serial.print("\n");
      }else{
        Serial.print(", ");
      }
    }
    Serial.println("End of packet\n");

    /*
    radio.stopListening();                                        // First, stop listening so we can talk   
    radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.      
    radio.startListening();                                       // Now, resume listening so we catch the next packets.     
    Serial.print(F("Sent response "));
    Serial.println(got_time);  
    */
    
  }
}
