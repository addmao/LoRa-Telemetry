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
#define DHTPIN 12    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22

// Define the SDI-12 bus
SDI12 sdi12Con(DATA_PIN);

// Define DHT type
DHT dht(DHTPIN, DHTTYPE);

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

struct dataStruct {
  uint16_t level;
  uint16_t temperature;
  uint16_t humidity;
} sensorData;

byte sender_data[sizeof(sensorData)] = {0};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  uint32_t ts = millis();
  while (!Serial) {
    if (millis() - ts > 10000) break;
  }
  digitalWrite(LED_BUILTIN, LOW);
  
  dht.begin();
  
  //Begin SDI12 connection
  sdi12Con.begin();
  delay(500);

  //Begin LoRa communication
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  //while (!Serial);
  delay(100);

  Serial.println("RID Y.20 LoRa Link");

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

void looptest() {
  sendWaterLeveltoLoRa(500);
  delay(5000);
}

uint8_t sdi12read(const char* command, char *response) {
  uint8_t index = 0;
  sdi12Con.sendCommand(command);
  delay(30);
  while (sdi12Con.available()) {
    char c = sdi12Con.read();
    if ((c!='\n') && (c!='\r')) {
      response[index++] = c;
      delay(10);
    }
  }
  response[index] = 0;
  return index;
}

float extractFirst(char* response) {
  // find the first + or -. If not found, return 0
  uint8_t i = 0;
  char *p;

  while (1) {
    char c = response[i++];
    if ((c == '+') || (c == '-'))
      break;
    else if (c == 0)
      return NAN;
  }
  p = response + i;

  // find the next + or - or the end of string
  while (1) {
    char c = response[i];
    if ((c == '+') || (c == '-') || (c == 0)) {
      response[i] = 0;
      break;
    }
    i++;
  }
  return atof(p);
}

void loop() {
  char response[128];
  uint8_t len;

  Serial.println("SDI12: start measurement");
  len = sdi12read("0M!",response);
  if (len > 1) {
    Serial.print("SDI12: response ");
    Serial.println(response);
  }

  delay(2000);
  Serial.println("SDI12: read measurement");
  len = sdi12read("0D0!",response);
  if (len > 1) {
    Serial.print("SDI12: response ");
    Serial.println(response);
    Serial.print("Extracted value: ");
    float value = extractFirst(response);
    Serial.println(value,4);
    sendWaterLeveltoLoRa((int)round(value*1000));
  }

  delay(10000);
}

void sendWaterLeveltoLoRa(uint16_t level) {
  sensorData.temperature = (uint16_t)dht.readTemperature();
  sensorData.humidity = (uint16_t)dht.readHumidity();
  sensorData.level = level;

  Serial.print("Data from SDI-12: ");
  Serial.println(sensorData.level);

  Serial.print("Sending Water Level: ");
  Serial.println(sensorData.level);
  Serial.print("Sending DHT data: Temp Humid ");
  Serial.print(sensorData.temperature);
  Serial.print(" ");
  Serial.println(sensorData.humidity);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);

  rf95.send((uint8_t*)&sensorData, sizeof(sensorData));
}
