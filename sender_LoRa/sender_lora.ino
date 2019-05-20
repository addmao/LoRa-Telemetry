#include <SPI.h>
#include <RH_RF95.h>

#include <SDI12.h>
#include <SDI12_boards.h>
#include <DHT.h>
#include <DHT_U.h>
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
int16_t index = 0;
int16_t i = 0;

struct dataStruct {
  uint16_t level;
  uint16_t temperature;
  uint16_t humidity;
} sensorData;

byte sender_data[sizeof(sensorData)] = {0};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  dht.begin();
  
  //Begin SDI12 connection
  sdi12_conn.begin();
  delay(500);
  sdi12_conn.forceListen();

  //Begin LoRa communication
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  //while (!Serial);
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
    RH_RF95::ModemConfig modem_config = {
      // See Table 86 for LoRa for more info
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x0c  // Reg 0x26: Mobile=On, Agc=On
    };
  //  rf95.setModemRegisters(&modem_config);

  //rf95.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  //  readSDI12Sensor();
  //  readDHT();
  //  sendDataToLoRa();
  //sendWaterLeveltoLoRa(121);

  int sdi12Available = sdi12_conn.available();
  if (sdi12Available < 0) {
    sdi12_conn.clearBuffer();
  }

  else if (sdi12Available > 0) {
    for (int a = 0; a < sdi12Available; a++) {
      sdi12_data = sdi12_conn.read();
      index++;
      if (index > 7 && sdi12_data != '\n') {
        sdi12_data_array[i] = sdi12_data;
        i++;
      }
      //      Serial.print("Index: ");
      //      Serial.println(index);
      //      Serial.print("i: ");
      //      Serial.println(i);

      if (index - i == 8) {
        sendWaterLeveltoLoRa(atoi(sdi12_data_array));
        delay(1000);
      }

      if (index - i >= 8) {
        index = 0;
        i = 0;
      }
    }
  }
}

bool sendWater = false;

void sendWaterLeveltoLoRa(uint16_t level) {
  //char* waterLevel_radiopacket;
  //float dht_radiopacket[2];
  //dht_radiopacket[0] = dht.readTemperature();
  //dht_radiopacket[1] = dht.readHumidity();
  //waterLevel_radiopacket = data;
  //SensorData sensorData;

  sensorData.temperature = (uint16_t)dht.readTemperature();
  sensorData.humidity = (uint16_t)dht.readHumidity();
  sensorData.level = level;

  Serial.print("Data from SDI-12: ");
  Serial.println(sensorData.level);

  Serial.print("Sending Water Level: ");
  Serial.println(sensorData.level);
  Serial.print("Sending DHT data: Temp Humid ");
  Serial.print(sensorData.temperature);b 
  Serial.print(" ");
  Serial.println(sensorData.humidity);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);

  rf95.send((uint8_t*)&sensorData, sizeof(sensorData));
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for water level complete reply...");
  if (rf95.waitAvailableTimeout(5000)) {
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    }
    else {
      Serial.println("Receive failed");
    }
  }
  else {
    Serial.println("No reply from another side, is there a listener around?");
  }
}
