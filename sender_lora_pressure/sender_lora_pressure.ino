#include <Adafruit_SleepyDog.h>

#include <SPI.h>
#include <RH_RF95.h>

#include <SDI12.h>
#include <SDI12_boards.h>
#include <DHT.h>
#include <DHT_U.h>
#include <string.h>

#include "pins.h"
#include "radio.h"
#include "packets.h"

//SDI12 definition
#define SERIAL_BAUD 9600   // The baud rate for the output serial port
#define DATA_PIN 11        // The pin of the SDI-12 data bus, checking pin with wisely
#define POWER_PIN -1       // The sensor power pin (or -1 if not switching power)

//DHT definition
#define DHTPIN 12    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22

#define WATCHDOG_TIMEOUT  (10*1000)
#define DEFAULT_COLLECT_INTERVAL 10  // secs

// Define the SDI-12 bus
SDI12 sdi12Con(DATA_PIN);

// Define DHT type
DHT dht(DHTPIN, DHTTYPE);

// Singleton instance of the radio driver

RH_RF95 rf95(PIN_RFM95_CS, PIN_RFM95_INT);

PacketData sensorData;

byte sender_data[sizeof(sensorData)] = {0};
uint16_t collect_interval = DEFAULT_COLLECT_INTERVAL;

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
  pinMode(PIN_RFM95_RST, OUTPUT);
  digitalWrite(PIN_RFM95_RST, HIGH);

  Serial.begin(9600);
  //while (!Serial);
  delay(100);

  Serial.println("RID Y.20 LoRa Link");

  if(radioInit(rf95)) {
	Serial.println("Radio init successful.");
  }

  else {
  	Serial.println("Radio init failed.");
  }
  pinMode(LED_BUILTIN, OUTPUT);
  Watchdog.enable(WATCHDOG_TIMEOUT);
}

void loop() {
  sendWaterLeveltoLoRa(500, 24, 13);
  uint32_t ts = millis();
  while(millis() - ts < collect_interval*1000) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    Watchdog.reset();
    if (!rf95.available())
      continue;
      
    if (!rf95.recv(buf,&len))
      continue;

    // a radio packet has been received; process it
    PacketHeader* header = (PacketHeader*)buf;
    if (header->type == PACKET_TYPE_CONFIG && len == sizeof(PacketConfig)) {
      PacketConfig *config = (PacketConfig*)buf;
      Serial.print("Receive config packet with interval = ");
      Serial.println(config->interval);
      // make sure the configured interval is in the valid range
      if (config->interval >= 10 && config->interval <= 5*60) {
        collect_interval = config->interval;
        Serial.println("New collect interval updated");
      }
      else {
        Serial.println("Interval value out of range; ignore");
      }
    }
  }
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

void looptest() {
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
    //sendWaterLeveltoLoRa((int)round(value*1000));
  }

  delay(10000);
}

void sendWaterLeveltoLoRa(uint16_t level, 
                          uint16_t water_temp,
                          uint16_t voltage) {
  //sensorData.header.type = 100;
  sensorData.level = level;
  sensorData.air_temp = dht.readTemperature()*100;
  sensorData.humidity = dht.readHumidity()*100;
  sensorData.water_temp = water_temp;
  sensorData.voltage = voltage;

  //Serial.print("Header Field: ");
  //Serial.println(sensorData.header.type);
  
  Serial.print("Sending Water Level: ");
  Serial.println(sensorData.level);
  Serial.print("Sending Water Temperature: ");
  Serial.println(sensorData.water_temp);
  Serial.print("Sending Air Temperature: ");
  Serial.print(sensorData.air_temp);
  Serial.print("Sending Air Humidity: ");
  Serial.print(sensorData.humidity);
  Serial.print("Sending Voltage Probe: ");
  Serial.println(sensorData.voltage);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);

  rf95.send((uint8_t*)&sensorData, sizeof(sensorData));
}
