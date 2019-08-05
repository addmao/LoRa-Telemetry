#include <Adafruit_SleepyDog.h>
#include <RH_RF95.h>
#include <SDI12.h>

#include "pt/pt.h"

#include "pins.h"
#include "radio.h"
#include "packets.h"

/**************************************************************************
 * If not zero, turn on debugging output over serial port
 */
#define DEBUG  1

/**************************************************************************
 * If not zero, wait for serial port connection after boot
 */
#define WAIT_FOR_SERIAL  0

/**************************************************************************
 * If greater than zero, force device to reset after the specified
 * milliseconds have passed
 *
 * @note
 * Due to 32-bit limitation of millis(), this value must not exceed 2**32 - 1
 */
#define FORCE_RESET_DURATION  (12UL*3600*1000) // msec

/**************************************************************************
 * Define the amount of time (in ms) allowed without resetting the watchdog
 * timer before triggering a watchdog reset
 */
#define WATCHDOG_TIMEOUT  (10*1000)   // msec

/**************************************************************************
 * Define the amount of time (in ms) allowed without receiving any data from a
 * sender before ignoring data requests from the logger
 *
 * @note
 * Due to 32-bit limitation of millis(), this value must not exceed 2**32 - 1
 */
#define RADIO_TIMEOUT      (1UL*60*1000) // msec

/////////////////////////////////////////
#if (DEBUG)
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

#define SDI12_MGMT_ADDR    "2"
#define SDI12_MGMT_DELAY    1
#define SDI12_MGMT_NUM_RESP 3

#define SHORT_BLINK(on,off) \
        do { \
          digitalWrite(PIN_LED,HIGH); \
          delay(on); \
          digitalWrite(PIN_LED,LOW); \
          delay(off); \
        } while (0);

#define PT_DELAY(pt,ms,tsVar) \
  tsVar = millis(); \
  PT_WAIT_UNTIL(pt, (uint32_t)(millis()-tsVar) >= (ms));

RH_RF95 rf95(PIN_RFM95_CS,PIN_RFM95_INT);
SDI12 sdi12Con(PIN_DATA);
PacketData packetData;
int16_t lastRssi;
bool data_available;
uint32_t lastReceived;
uint8_t remote_heartbeat, local_heartbeat;
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
void force_reset() {
  Watchdog.enable(1000);
  while (1)
    ;
}

/*****************************************/
void setup()
{
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_RFM95_RST,INPUT);
  delay(1000);
  digitalWrite(PIN_LED,HIGH);

#if (DEBUG)
  Serial.begin(9600);
#if (WAIT_FOR_SERIAL)
  while (!Serial)
    ;
#endif
  DEBUG_PRINTLN(F("receiver starting up..."));
#endif

  memset(&packetData,0,sizeof(packetData));
  data_available = false;
  lastReceived = 0;
  remote_heartbeat = 0;
  local_heartbeat = 0;

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
#if (FORCE_RESET_DURATION > 0)
  if (millis() > FORCE_RESET_DURATION)
    force_reset();
#endif
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
        remote_heartbeat++;
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
      if (data_available) {
        sprintf(response,SDI12_WATER_ADDR "%03d%d\r\n",
          SDI12_WATER_DELAY,
          SDI12_WATER_NUM_RESP);
        DEBUG_PRINT("Sending response: ");
        DEBUG_PRINTLN(response);
        sdi12Con.sendResponse(response);
        SHORT_BLINK(50,0);
      }
      else {
        DEBUG_PRINTLN("Data not available; ignore request.");
      }
    }
    else if (!strcmp(sdibuf,SDI12_WATER_ADDR "D0")) {
      if (data_available) {
        // (1) water-level,
        // (2) water-temp,
        // (3) probe-voltage
        sprintf(response,SDI12_WATER_ADDR "+%d.%03d+%d.%01d+%d.%01d\r\n",
          packetData.level/1000,
          packetData.level%1000,
          packetData.water_temp/10,
          packetData.water_temp%10,
          packetData.voltage/10,
          packetData.voltage%10);
        DEBUG_PRINT("Sending response: ");
        DEBUG_PRINTLN(response);
        sdi12Con.sendResponse(response);
        SHORT_BLINK(50,0);
      }
      else {
        DEBUG_PRINTLN("Data not available; ignore request.");
      }
    }

    else if (!strcmp(sdibuf,SDI12_DHT_ADDR "M")) {
      if (data_available) {
        sprintf(response,SDI12_DHT_ADDR "%03d%d\r\n",
          SDI12_DHT_DELAY,
          SDI12_DHT_NUM_RESP);
        DEBUG_PRINT("Sending response: ");
        DEBUG_PRINTLN(response);
        sdi12Con.sendResponse(response);
        SHORT_BLINK(50,0);
      }
      else {
        DEBUG_PRINTLN("Data not available; ignore request.");
      }
    }
    else if (!strcmp(sdibuf,SDI12_DHT_ADDR "D0")) {
      if (data_available) {
        // (1) air-temp,
        // (2) air-humidity,
        // (3) last-rssi
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
      else {
        DEBUG_PRINTLN("Data not available; ignore request.");
      }
    }

    else if (!strcmp(sdibuf,SDI12_MGMT_ADDR "M")) {
      sprintf(response,SDI12_MGMT_ADDR "%03d%d\r\n",
        SDI12_MGMT_DELAY,
        SDI12_MGMT_NUM_RESP);
      DEBUG_PRINT("Sending response: ");
      DEBUG_PRINTLN(response);
      sdi12Con.sendResponse(response);
      SHORT_BLINK(50,0);
    }
    else if (!strcmp(sdibuf,SDI12_MGMT_ADDR "D0")) {
      // (1) local-heartbeat,
      // (2) remote-heartbeat,
      // (3) seconds since last heard
      sprintf(response,SDI12_MGMT_ADDR "+%u+%u+%lu\r\n",
        local_heartbeat,
        remote_heartbeat,
        (uint32_t)(millis()-lastReceived) / 1000);
      DEBUG_PRINT("Sending response: ");
      DEBUG_PRINTLN(response);
      sdi12Con.sendResponse(response);
      SHORT_BLINK(50,0);
      local_heartbeat++;
    }
  }

  PT_END(pt);
}

/*****************************************/
PT_THREAD(taskTimer(struct pt* pt)) {

  static uint32_t ts;

  PT_BEGIN(pt);

  for (;;) {
    if (data_available && (uint32_t)(millis()-lastReceived) >= RADIO_TIMEOUT) {
      DEBUG_PRINTLN("Radio reception timed out");
      data_available = false;
    }
    // perform check every 5 seconds
    PT_DELAY(pt,5000,ts);
  }

  PT_END(pt);
}
