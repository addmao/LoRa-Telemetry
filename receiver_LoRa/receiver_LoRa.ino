// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_SleepyDog.h>

#include "pins.h"
#include "radio.h"
#include "packets.h"

#define WATCHDOG_TIMEOUT  (10*1000)

//RH_RF95 rf95(RFM95_CS, RFM95_INT);
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13

PacketData receiveData;
//struct dataStruct{
  //uint16_t checkingField; //MAGIC FIELD
  //uint16_t level;
  //uint16_t air_temp;
  //uint16_t humidity;
  //uint16_t water_temp;
  //uint16_t voltage;
//} receiveData;


void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  while (!Serial) {
    //delay(1);
  }
  delay(100);

  if(radioInit(rf95_driver)) {
    Serial.prinln("Radio init successful.");
  }

  else {
    Serial.println("Radio init failed.");
  }
  Watchdog.enable(WATCHDOG_TIMEOUT);
}

void loop()
{
  if (rf95_driver.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if(rf95_driver.recv(buf, &len)) {
     PacketHeader* header = (PacketHeader*) buf;
     if(header->type == PACKET_TYPE_DATA && len == sizeof(PacketData)) {
       digitalWrite(LED, HIGH);

       memcpy(&receiveData, buf, len);

       Serial.print(receiveData.air_temp); // Box Temperature
       Serial.print(",");
       Serial.print(receiveData.humidity/1000); // Box Humidity
       Serial.print(",");
       Serial.print(rf95_driver.lastRssi(), DEC); // RSSI
       Serial.print(",");
       Serial.print(rf95_driver.lastSNR()); // SNR
       Serial.print(",");
       Serial.print(receiveData.level); // Water Level
       Serial.print(",");
       Serial.print(receiveData.water_temp/1000);
       Serial.print(",");
       Serial.print(receiveData.voltage);
       Serial.print(",");
      
       digitalWrite(LED, LOW);
    }

    else {
      Serial.println("Receive failed");
    }
  }
  Watchdog.reset();
}
