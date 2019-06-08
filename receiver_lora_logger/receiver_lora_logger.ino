#include <Adafruit_SleepyDog.h>
#include <RH_RF95.h>
#include <SDI12.h>

#include "pt/pt.h"

#define DEBUG
#define WATCHDOG_TIMEOUT  (10*1000)   // msec
#define DATA_TIMEOUT      (5*60*1000) // msec

#ifdef DEBUG
#define DEBUG_PRINT(x...) Serial.print(x)
#define DEBUG_PRINTLN(x...) Serial.println(x)
#else
#define DEBUG_PRINT(x...)
#define DEBUG_PRINTLN(x...)
#endif

#define PIN_LED   LED_BUILTIN
#define PIN_DATA  11

/* for Feather32u4 RFM9x */
#define PIN_RFM95_CS  8
#define PIN_RFM95_RST 4
#define PIN_RFM95_INT 7

/*for Feather M0 RFM9x
#define PIN_RFM95_CS 8
#define PIN_RFM95_RST 4
#define PIN_RFM95_INT 3
*/

#define SDI12_ADDR     "0"
#define SDI12_DELAY    2
#define SDI12_NUM_RESP 3

#define RF95_FREQ 434.0

#define SHORT_BLINK(on,off) \
        do { \
          digitalWrite(LED_BUILTIN,HIGH); \
          delay(on); \
          digitalWrite(LED_BUILTIN,LOW); \
          delay(off); \
        } while (0);

#define PT_DELAY(pt,ms,tsVar) \
  tsVar = millis(); \
  PT_WAIT_UNTIL(pt, millis()-tsVar >= (ms));

struct SensorData {
  uint16_t level;
  uint16_t air_temp;
  uint16_t humidity;
  uint16_t water_temp;
  uint16_t voltage;
};

RH_RF95 rf95(PIN_RFM95_CS,PIN_RFM95_INT);
SDI12 sdi12Con(PIN_DATA);
SensorData sensorData;
bool data_available;
uint32_t lastReceived;
struct pt ptRadio;
struct pt ptSdi;
struct pt ptTimer;

/*****************************************/
void error_blink_loop(uint8_t code) {
  DEBUG_PRINT(F("ERROR: code "));
  DEBUG_PRINTLN(code);
  for (;;) {
    SHORT_BLINK(200,200);
  }
}

/*****************************************/
void setup()
{
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_RFM95_RST,INPUT);

#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial)
    ;
  DEBUG_PRINTLN(F("receiver starting up..."));
#endif

  memset(&sensorData,0,sizeof(sensorData));
  data_available = false;
  lastReceived = 0;

  // manual reset
  // should RST be left floating (per datasheet)?
  digitalWrite(PIN_RFM95_RST,LOW);
  pinMode(PIN_RFM95_RST,OUTPUT);
  delay(1);
  pinMode(PIN_RFM95_RST,INPUT);
  delay(10);

  while (!rf95.init()) {
    DEBUG_PRINTLN("LoRa radio init failed");
    error_blink_loop(1);
  }

  if (!rf95.setFrequency(RF95_FREQ)) {
    DEBUG_PRINTLN("LoRa radio set frequency failed");
    error_blink_loop(2);
  }

  rf95.setTxPower(23,false);
  RH_RF95::ModemConfig modem_config = {
      // See Table 86 for LoRa for more info
      0x78, // Reg 0x1d: BW=125kHz, Coding=4/8, Header=explicit
      0xc4, // Reg 0x1e: Spread=4096chips/symbol, CRC=enable
      0x0c  // Reg 0x26: Mobile=On, Agc=On
    };
  rf95.setModemRegisters(&modem_config);

  DEBUG_PRINTLN("Radio initialized.");

  sdi12Con.begin();
  delay(500);
  sdi12Con.forceListen();
  DEBUG_PRINTLN("SDI-12 initialized");

  PT_INIT(&ptRadio);
  PT_INIT(&ptSdi);
  PT_INIT(&ptTimer);

  Watchdog.enable(WATCHDOG_TIMEOUT);
}

/*****************************************/
void loop()
{ 
  taskRadio(&ptRadio);
  taskSdi(&ptSdi);
  taskTimer(&ptTimer);
  Watchdog.reset();
}

/*****************************************/
PT_THREAD(taskRadio(struct pt* pt)) {

  static uint32_t ts;

  PT_BEGIN(pt);

  for (;;) {
    PT_WAIT_UNTIL(pt,rf95.available());
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf,&len)) {
      if (len == sizeof(sensorData)) {
        DEBUG_PRINTLN("Radio data received.");
        memcpy(&sensorData,buf,len);
        SHORT_BLINK(50,100);
        SHORT_BLINK(50,0);
        lastReceived = millis();
        data_available = true;
      }
    }
  }

  PT_END(pt);
}

/*****************************************/
PT_THREAD(taskSdi(struct pt* pt)) {

  static char sdibuf[128];
  static uint32_t ts;
  static uint8_t index;
  char response[128];

  PT_BEGIN(pt);

  for (;;) {
    // collect SDI command into the buffer until a newline is encountered
    if (sdi12Con.available() < 0)
      sdi12Con.clearBuffer();
    index = 0;
    for (;;) {
      PT_WAIT_UNTIL(pt, sdi12Con.available() > 0);
      char c = sdi12Con.read();
      if (c == '!')
        break;
      else if (c != 0)
        sdibuf[index++] = c;
      if (index >= sizeof(sdibuf)-1) // prevent buffer overflow
        break;
    }
    sdibuf[index] = 0;
    sdi12Con.clearBuffer();
    DEBUG_PRINT("Receive SDI-12 command: ");
    DEBUG_PRINTLN(sdibuf);

    if (!data_available) {
      DEBUG_PRINTLN("Data unavailable; ignore request.");
      continue;
    }

    // XXX for testing
    //sensorData.level = 1800;
    //sensorData.voltage = 124;
    //sensorData.water_temp = 321;

    if (!strcmp(sdibuf,SDI12_ADDR "M")) {
      sprintf(response,SDI12_ADDR "%03d%d\r\n",
        SDI12_DELAY,
        SDI12_NUM_RESP);
      // addr => 0, measure time => 2, num responses => 3
      DEBUG_PRINT("Sending response: ");
      DEBUG_PRINTLN(response);
      sdi12Con.sendResponse(response);
      SHORT_BLINK(50,0);
    }
    else if (!strcmp(sdibuf,SDI12_ADDR "D0")) {
      sprintf(response,SDI12_ADDR "+%d.%03d+%d.%d+%d.%d\r\n",
        sensorData.level/1000,
        sensorData.level%1000,
        sensorData.water_temp/10,
        sensorData.water_temp%10,
        sensorData.voltage/10,
        sensorData.voltage%10);
      DEBUG_PRINT("Sending response: ");
      DEBUG_PRINTLN(response);
      sdi12Con.sendResponse(response);
      SHORT_BLINK(50,0);
    }
  }

  PT_END(pt);
}

/*****************************************/
PT_THREAD(taskTimer(struct pt* pt)) {

  static uint32_t ts;

  PT_BEGIN(pt);

  for (;;) {
    // TODO: take care of wrap-around
    if (data_available && (uint32_t)(millis()-lastReceived) >= DATA_TIMEOUT) {
      memset(&sensorData,0,sizeof(sensorData));
      DEBUG_PRINTLN("Radio reception timed out; clear sensor data");
    }
    PT_DELAY(pt,5000,ts);
  }

  PT_END(pt);
}
