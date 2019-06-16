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

/****************************************************************************
 *
 */
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

/****************************************************************************
 *
 */
void loop() {
  char response[128];
  char field[10];
  uint8_t len;

  /////////////////////////////////////////////////////////////////
  // read values from the SDI-12 probe and transmit over LoRa
  /////////////////////////////////////////////////////////////////
  Serial.println("SDI12: start measurement");
  len = sdi12read("0M!",response);
  if (len > 1) {
    Serial.print("SDI12: response ");
    Serial.println(response);
  }

  // ignore the returned measurement time for now and use a fixed 2-second
  // delay
  for (uint8_t i=0; i<2; i++) {
    delay(1000);
    Watchdog.reset();
  }
  Serial.println("SDI12: read measurement");
  len = sdi12read("0D0!",response);
  if (len > 1) {
    Serial.print("SDI12: response ");
    Serial.println(response);
    char* p = response + 1; // skip the address field
    p = extractField(p,field,sizeof(field),true); // water level
    sensorData.level = atoi(field);
    p = extractField(p,field,sizeof(field),true); // water temperature
    sensorData.water_temp = atoi(field);
    p = extractField(p,field,sizeof(field),true); // voltage
    sensorData.voltage = atoi(field);

    // obtain air temperature and humidity from DHT-22
    sensorData.air_temp = (uint16_t)(dht.readTemperature()*100);
    sensorData.humidity = (uint16_t)(dht.readHumidity()*100);

    sendToLoRa(sensorData);
  }

  /////////////////////////////////////////////////////////////////
  // wait for collect_interval seconds before the next measurement
  // and keep checking radio for config packet while waiting
  /////////////////////////////////////////////////////////////////
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

/****************************************************************************
 * Issue a command to SDI-12 sensor and stores the response in the given
 * buffer
 */
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

/****************************************************************************
 * Extract a value field from SDI-12 response.
 * The result is kept in result and pointer to the next field is returned.
 * The len value indicates the maximum length of the result buffer, including
 * the terminating null.  If ignore_point is true, all decimal points are
 * ignored, which makes the behavior similar to Unidata's loggers.  (Refer to
 * the section SDI: SDI Transducer in the Unidata's manual.)
 */
char* extractField(char* response, char* result, uint8_t len, bool ignore_point) {
  uint8_t n = 0;
  while (*response && n < len-1) {
    if (ignore_point && *response == '.')
      response++;
    *result++ = *response++;
    n++;
    if (*response == '+' || *response == '-')
      break;
  }
  *result = 0;
  return response;
}

/****************************************************************************
 * Send 
 */
void sendToLoRa(PacketData& data) {
  //Serial.print("Header Field: ");
  //Serial.println(sensorData.header.type);
  
  Serial.print("Sending Water Level: ");
  Serial.println(data.level);
  Serial.print("Sending Water Temperature: ");
  Serial.println(data.water_temp);
  Serial.print("Sending Air Temperature: ");
  Serial.print(data.air_temp);
  Serial.print("Sending Air Humidity: ");
  Serial.print(data.humidity);
  Serial.print("Sending Voltage Probe: ");
  Serial.println(data.voltage);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);

  rf95.send((uint8_t*)&data, sizeof(PacketData));
}
