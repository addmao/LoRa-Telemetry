#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <RH_RF95.h>

#include "printf.h"
#include "pt/pt.h"

#include "pins.h"
#include "radio.h"
#include "packets.h"

#define TIMEOUT               5000  // timeout in ms
#define TIMER_START(ts)       ts = millis();
#define TIMER_EXPIRED(ts,to)  (millis()-ts > to)

#define SHORT_BLINK(on,off) \
        do { \
          digitalWrite(LED_BUILTIN,HIGH); \
          delay(on); \
          digitalWrite(LED_BUILTIN,LOW); \
          delay(off); \
        } while (0);

struct pt ptInteract;
PT_THREAD(interactTask(struct pt* pt));
RH_RF95 rf95(PIN_RFM95_CS,PIN_RFM95_INT);
bool listening;

/***********************************************
 * 
 */
void error_blink_loop(uint8_t code) {
  Serial.print(F("ERROR: code "));
  Serial.println(code);
  for (;;) {
    SHORT_BLINK(200,200);
  }
}

/***********************************************
 * 
 */
void setup() {
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_RFM95_RST,INPUT);
  Serial.begin(9600);
  while (!Serial)
    ;
  if (radioInit(rf95)) {
    Serial.println("Radio init successful.");
  }
  else {
    Serial.println("Radio init failed.");
    error_blink_loop(1);
  }

  listening = false;
  PT_INIT(&ptInteract);
}

/***********************************************
 * 
 */
void loop() {
  interactTask(&ptInteract);

  if (rf95.available()) {
    PacketHeader *header;
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf,&len) && listening) {
      header = (PacketHeader*) buf;
      if (header->type == PACKET_TYPE_DATA && len == sizeof(PacketData)) {
        PacketData* data = (PacketData*) buf;
        printf(Serial,F("\r\n"));
        printf(Serial,F("* RX: DATA (RSSI = %d dBm)\r\n"), rf95.lastRssi());
        printf(Serial,F("*  level = %d\r\n"), data->level);
        printf(Serial,F("*  air_temp = %d\r\n"), data->air_temp);
        printf(Serial,F("*  humidity = %d\r\n"), data->humidity);
        printf(Serial,F("*  water_temp = %d\r\n"), data->water_temp);
        printf(Serial,F("*  voltage = %d\r\n"), data->voltage);
      }
      else {
        printf(Serial,F("\r\n"));
        printf(Serial,F("* RX: Unknown (RSSI = %d dBm)\r\n"), rf95.lastRssi());
        printf(Serial,F("*  type = %d\r\n"), header->type);
        printf(Serial,F("*  len = %d\r\n"), len);
      }
    }
  }
}

/***********************************************
 * 
 */
PT_THREAD(interactTask(struct pt* pt)) {

  static char cmdline[60];
  static char* ptr;

  PT_BEGIN(pt);

  for (;;) {
    Serial.println();
    Serial.println(F("Menu"));
    Serial.println(F("===="));
    Serial.println(F("i <interval> - set update interval (in seconds)"));
    printf(Serial, F("l - toggle listen mode (currently %s)\r\n"), listening ? "ON" : "OFF");
    Serial.print(F(">>> "));

    // read a command line
    ptr = cmdline;
    for (;;) {
      PT_WAIT_UNTIL(pt, Serial.available());
      char c = Serial.read();
      Serial.write(c);
      if (c == '\r' || c == '\n')
      {
        *ptr = 0;
        while (Serial.available())
          Serial.read(); // flush serial input
        Serial.println();
        break;
      }
      else
        *ptr++ = c;

      if (ptr-cmdline == sizeof(cmdline)) { // prevent buffer overflow
        *ptr = 0;
        break;
      }
    }

    // decode the command
    char *token, *last, cmd;
    token = strtok_r(cmdline," ",&last);
    if (token == NULL)
      continue;  // empty command
    cmd = token[0];
    if (strlen(token) != 1) {
      Serial.println(F("Invalid command!"));
      continue;
    }

    if (cmd == 'i') {
      // extract the addr
      token = strtok_r(NULL," ",&last);
      if (token == NULL) {
          Serial.println(F("Missing interval"));
          continue;
      }
      PacketConfig config;
      config.interval = atoi(token);
      rf95.send((uint8_t*)&config, sizeof(config));
    }
    else if (cmd == 'l') {
      listening ^= true;
    }
    else {
      Serial.println(F("Invalid command!"));
    }
  }

  PT_END(pt);
}
