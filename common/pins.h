#ifndef __PINS_H__
#define __PINS_H__

#define PIN_LED   LED_BUILTIN
#define PIN_DATA  11

#if defined(ARDUINO_AVR_FEATHER32U4)
#define PIN_RFM95_CS  8
#define PIN_RFM95_RST 4
#define PIN_RFM95_INT 7

#elif defined(ARDUINO_SAMD_FEATHER_M0)
#define PIN_RFM95_CS 8
#define PIN_RFM95_RST 4
#define PIN_RFM95_INT 3

#else
#error("Unsupported board")
#endif

#endif
