// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <RHReliableDatagram.h>
#include <SPI.h>
#include <RH_RF95.h>

/* for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
*/

/*for feather m0 RFM9x*/
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

/* for shield 
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 7
*/

/* Feather 32u4 w/wing
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)
*/

/* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/

#if defined(ESP8266)
  /* for ESP w/featherwing */ 
  #define RFM95_CS  2    // "E"
  #define RFM95_RST 16   // "D"
  #define RFM95_INT 15   // "B"

#elif defined(ESP32)  
  /* ESP32 feather w/wing */
  #define RFM95_RST     27   // "A"
  #define RFM95_CS      33   // "B"
  #define RFM95_INT     12   //  next to A

#elif defined(NRF52)  
  /* nRF52832 feather w/wing */
  #define RFM95_RST     7   // "A"
  #define RFM95_CS      11   // "B"
  #define RFM95_INT     31   // "C"
  
#elif defined(TEENSYDUINO)
  /* Teensy 3.x w/wing */
  #define RFM95_RST     9   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     4    // "C"
#endif

#define RF95_FREQ 434.0

// Singleton instance of the radio driver
//RH_RF95 rf95(RFM95_CS, RFM95_INT);
RH_RF95 rf95_driver(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial) {
    //delay(1);
  }
  delay(100);

  //Serial.println("Feather LoRa RX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95_driver.init()) {
    //Serial.println("LoRa radio init failed");
    while (1);
  }

  if (!rf95_driver.setFrequency(RF95_FREQ)) {
    //Serial.println("setFrequency failed");
    while (1);
  }

  rf95_driver.setFrequency(434.0);
  rf95_driver.setTxPower(23, false);
  rf95_driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
}

uint8_t data_ack[] = "Received package";

void loop()
{ 
  if (rf95_driver.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    uint8_t from;
    
    if (rf95_driver.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      //Serial.print("Water Level: ");
      if(len == 8) {
        //DHT
        //Serial.println("DHT");
        float* dht_data_receive = (float*)buf;
        Serial.print(dht_data_receive[0]);//Temperature
        Serial.print(",");
        Serial.print(dht_data_receive[1]);//Humidity
        Serial.print(",");
        Serial.print(rf95_driver.lastRssi(), DEC);
        Serial.print(",");
      }

      else if(len == 20) {
        //WATER_LEVEL
        //Serial.println("WATER");
        char* water_level_data_receive = (char*)buf;
        Serial.println(water_level_data_receive);
      }
      //Serial.print(",");
      //Serial.print("RSSI: ");
      //Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      rf95_driver.send(data_ack, sizeof(data_ack));
      rf95_driver.waitPacketSent();
      //Serial.println("Sent a reply");
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  //Serial.flush();
}
