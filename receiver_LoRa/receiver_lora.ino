// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

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

struct dataStruct{
  uint16_t checkingField; //MAGIC FIELD
  uint16_t level;
  uint16_t air_temp;
  uint16_t humidity;
  uint16_t water_temp;
  uint16_t voltage;
} receiveData;


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
  RH_RF95::ModemConfig modem_config = {
    // See Table 86 for LoRa for more info
    0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
    0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
    0x0c  // Reg 0x26: Mobile=On, Agc=On
  };
  rf95_driver.setModemRegisters(&modem_config);
//rf95_driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
}

void loop()
{
  if (rf95_driver.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    uint8_t from;

    if (rf95_driver.recv(buf, &len) && receiveData.checkingField == 100)
    {
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
    else
    {
      Serial.println("Receive failed");
    }
  }
}
