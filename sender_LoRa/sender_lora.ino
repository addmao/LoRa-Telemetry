#include <RHReliableDatagram.h>
#include <SPI.h>
#include <RH_RF95.h>

#include <SDI12.h>
#include <SDI12_boards.h>
#include <DHT.h>
#include <DHT_U.h>

#include <EEPROM.h>
#include <string.h>

//BEGIN RADIO PART
/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

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

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0
//END RADIO PART

//SDI12 definition
#define SERIAL_BAUD 9600   // The baud rate for the output serial port
#define DATA_PIN 11        // The pin of the SDI-12 data bus, checking pin with wisely
#define POWER_PIN -1       // The sensor power pin (or -1 if not switching power)

//DHT definition
#define dataPin 8
#define DHTPIN 13    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 

// Define the SDI-12 bus
SDI12 sdi12_conn(DATA_PIN);

// Define DHT type 
DHT dht(DHTPIN, DHTTYPE);

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

char sdi12_data;
char sdi12_data_array[128];
uint8_t sdiBufPtr;
int16_t packetnum = 0;  // packet counter, we increment per xmission
int16_t index = 0;
int16_t i = 0;

void setup() {
  //Begin SDI12 connection
  sdi12_conn.begin();
  delay(500);
  sdi12_conn.forceListen();
  
  //Begin LoRa communication
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial);
  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }

  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
 // rf95_driver.setFrequency(RF95_FREQ)
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }


  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);
  Serial.begin(SERIAL_BAUD);
}

void loop() {
  //if(rf95_manager.available()) {
    int sdi12Available = sdi12_conn.available();
    if(sdi12Available < 0) {
      sdi12_conn.clearBuffer();
    }
  
    else if(sdi12Available > 0) {
      for (int a=0; a<sdi12Available; a++) {  
        sdi12_data = sdi12_conn.read();
        index++;
        if(index > 7 && sdi12_data != '\n') {
          sdi12_data_array[i] = sdi12_data;
          i++; 
        }
  //      Serial.print("Index: ");
  //      Serial.println(index);
  //      Serial.print("i: ");
  //      Serial.println(i);
        
        if (index-i == 8) {
          sendWaterLeveltoLoRa(sdi12_data_array);
          //writeDataToEEPROM();
        }
  
        if(index - i >= 8) {
          index = 0;
          i = 0;
        }
      }
     }
  //}
}

//BEGIN EEPROM Programming part
//source code from EEPROM examples in Arduino IDE
void readDataFromEEPROM() {
  float f = 0.00f;   //Variable to store data read from EEPROM.
  unsigned int eeAddress = 0; //EEPROM address to start reading from

  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.print("Read float from EEPROM: ");

  EEPROM.get(eeAddress, f);
  Serial.println(f, 3);  //This may print 'ovf, nan' if the data inside the EEPROM is not a valid float.
  secondTest(); //Run the next test.  
}

void writeDataToEEPROM() {
  unsigned int addr = 0;

  char val = 'A';

  Serial.print(val);
  delay(5000);
  
  EEPROM.write(addr, val);
  addr = addr + 1;
  if(addr == EEPROM.length())
    addr = 0;
    
  delay(100);  
}

void clearEEPROM() {
  for (unsigned int i = 0 ; i < EEPROM.length() ; i++)
    EEPROM.write(i, 0);
  digitalWrite(13, HIGH);
}

struct MyObject{
  float field1;
  byte field2;
  char name[10];
};

void secondTest(){
  int eeAddress = sizeof(float); //Move address to the next byte after float 'f'.

  MyObject customVar; //Variable to store custom object read from EEPROM.
  EEPROM.get( eeAddress, customVar );

  Serial.println( "Read custom object from EEPROM: " );
  Serial.println( customVar.field1 );
  Serial.println( customVar.field2 );
  Serial.println( customVar.name );
}
//END EEPROM Programming part

void sendWaterLeveltoLoRa(char data[]) {
  delay(1000);
  char* waterLevel_radiopacket;
  float dht_radiopacket[2];
  dht_radiopacket[0] = dht.readTemperature();
  dht_radiopacket[1] = dht.readHumidity();
  waterLevel_radiopacket = data;

  Serial.print("Data from SDI-12: ");
  Serial.println(waterLevel_radiopacket);
  delay(2000);

  //itoa(packetnum++, waterLevel_radiopacket, 10);
  Serial.print("Sending Water Level: "); 
  Serial.println(waterLevel_radiopacket);
  Serial.print("Sending DHT data: Temp Humid "); 
  Serial.print(dht_radiopacket[0]);
  Serial.print(" ");
  Serial.println(dht_radiopacket[1]);
  //rf95.send((uint8_t*)waterLevel_radiopacket, 20);
  
  //delay(2000);//Delay Before Sending dht LoRa
  rf95.send((uint8_t*)dht_radiopacket, 8);
  
  delay(2000);//Delay Before Sending water level LoRa
  rf95.send((uint8_t*)waterLevel_radiopacket, 20);

  //rf95.waitPacketSent();
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for water level complete reply...");
  if (rf95.waitAvailableTimeout(5000)) { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len)){
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC); 
      //waterLevel_radiopacket = "";
    }
    else {
      Serial.println("Receive failed");
    }
  }
  else {
    Serial.println("No reply from another side, is there a listener around?");
  }
}

//calculate distance from rssi
// double getDistance(int rssi, int txPower) {
//    /*
//     * RSSI = TxPower - 10 * n * lg(d)
//     * n = 2 (in free space)
//     * 
//     * d = 10 ^ ((TxPower - RSSI) / (10 * n))
//     */
// 
//    return Math.pow(10d, ((double) txPower - rssi) / (10 * 2));
//}
