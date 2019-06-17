#include <Adafruit_SleepyDog.h>
#include <RH_RF95.h>
#include <SDI12.h>

#include "pt/pt.h"

#include "pins.h"
#include "radio.h"
#include "packets.h"

#define DEBUG
#define WATCHDOG_TIMEOUT  (10*1000)   // msec
#define DATA_TIMEOUT      (1L*60*1000) // msec

#ifdef DEBUG
#define DEBUG_PRINT(x...) Serial.print(x)
#define DEBUG_PRINTLN(x...) Serial.println(x)
#else
#define DEBUG_PRINT(x...)
#define DEBUG_PRINTLN(x...)
#endif

#define SDI12_WATER_ADDR    "0"
#define SDI12_WATER_DELAY    2
#define SDI12_WATER_NUM_RESP 3

#define SDI12_DHT_ADDR    "1"
#define SDI12_DHT_DELAY    1
#define SDI12_DHT_NUM_RESP 3

#define SHORT_BLINK(on,off) \
        do { \
          digitalWrite(PIN_LED,HIGH); \
          delay(on); \
          digitalWrite(PIN_LED,LOW); \
          delay(off); \
        } while (0);

#define PT_DELAY(pt,ms,tsVar) \
  tsVar = millis(); \
  PT_WAIT_UNTIL(pt, millis()-tsVar >= (ms));

RH_RF95 rf95(PIN_RFM95_CS,PIN_RFM95_INT);
SDI12 sdi12Con(PIN_DATA);
PacketData packetData;
int16_t lastRssi;
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
  delay(1000);
  digitalWrite(PIN_LED,HIGH);

#ifdef DEBUG
  Serial.begin(9600);
  //while (!Serial)
  //  ;
  DEBUG_PRINTLN(F("receiver starting up..."));
#endif

  memset(&packetData,0,sizeof(packetData));
  data_available = false;
  lastReceived = 0;

  if (radioInit(rf95)) {
    DEBUG_PRINTLN("Radio init successful.");
  }
  else {
    DEBUG_PRINTLN("Radio init failed.");
    error_blink_loop(1);
  }

  sdi12Con.begin();
  delay(500);
  sdi12Con.forceListen();
  DEBUG_PRINTLN("SDI-12 initialized");

  PT_INIT(&ptRadio);
  PT_INIT(&ptSdi);
  PT_INIT(&ptTimer);

  Watchdog.enable(WATCHDOG_TIMEOUT);
  digitalWrite(PIN_LED,LOW);
}

/*****************************************/
void loop() { 
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
      PacketHeader* header = (PacketHeader*)buf;
      if (header->type == PACKET_TYPE_DATA && len == sizeof(PacketData)) {
        DEBUG_PRINTLN("Radio data received.");
        memcpy(&packetData,buf,len);
        SHORT_BLINK(50,100);
        SHORT_BLINK(50,0);
        lastReceived = millis();
        data_available = true;
        lastRssi = rf95.lastRssi();
      }
      else {
        DEBUG_PRINT("Unknown radio packet type ");
        DEBUG_PRINT(header->type);
        DEBUG_PRINT(" and len ");
        DEBUG_PRINT(len);
        DEBUG_PRINTLN("; ignore.");
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
    // packetData.level = 1856;
    // packetData.voltage = 124;
    // packetData.water_temp = 283;
    //
    // Water level displayed on StarLogV4:
    // 756 -> -30.401
    // 1756 -> -15.201
    // 1800 -> 26.217
    // 1856 -> -28.309

    if (!strcmp(sdibuf,SDI12_WATER_ADDR "M")) {
      sprintf(response,SDI12_WATER_ADDR "%03d%d\r\n",
        SDI12_WATER_DELAY,
        SDI12_WATER_NUM_RESP);
      DEBUG_PRINT("Sending response: ");
      DEBUG_PRINTLN(response);
      sdi12Con.sendResponse(response);
      SHORT_BLINK(50,0);
    }
    else if (!strcmp(sdibuf,SDI12_WATER_ADDR "D0")) {
      sprintf(response,SDI12_WATER_ADDR "+%d.%03d+%d.%02d+%d.%02d\r\n",
        packetData.level/1000,
        packetData.level%1000,
        packetData.water_temp/100,
        packetData.water_temp%100,
        packetData.voltage/100,
        packetData.voltage%100);
      DEBUG_PRINT("Sending response: ");
      DEBUG_PRINTLN(response);
      sdi12Con.sendResponse(response);
      SHORT_BLINK(50,0);
    }

    else if (!strcmp(sdibuf,SDI12_DHT_ADDR "M")) {
      sprintf(response,SDI12_DHT_ADDR "%03d%d\r\n",
        SDI12_DHT_DELAY,
        SDI12_DHT_NUM_RESP);
      DEBUG_PRINT("Sending response: ");
      DEBUG_PRINTLN(response);
      sdi12Con.sendResponse(response);
      SHORT_BLINK(50,0);
    }
    else if (!strcmp(sdibuf,SDI12_DHT_ADDR "D0")) {
      sprintf(response,SDI12_DHT_ADDR "+%d.%02d+%d.%02d%c%d\r\n",
        packetData.air_temp/100,
        packetData.air_temp%100,
        packetData.humidity/100,
        packetData.humidity%100,
        lastRssi < 0 ? '-' : '+',
        abs(lastRssi));
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
      DEBUG_PRINTLN("Radio reception timed out");
      data_available = false;
    }
    PT_DELAY(pt,5000,ts);
  }

  PT_END(pt);
}
