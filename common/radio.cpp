#include <RH_RF95.h>
#include "radio.h"
#include "pins.h"

bool radioInit(RH_RF95& rf95) {
  // manual reset
  // should RST be left floating (per datasheet)?
  digitalWrite(PIN_RFM95_RST,LOW);
  pinMode(PIN_RFM95_RST,OUTPUT);
  delay(1);
  pinMode(PIN_RFM95_RST,INPUT);
  delay(10);

  if (!rf95.init())
    return false;

  if (!rf95.setFrequency(RADIO_FREQ))
    return false;

  rf95.setTxPower(RADIO_TX_POWER,false);
  RH_RF95::ModemConfig modem_config = {
      // See Table 86 for LoRa for more info
      0x78, // Reg 0x1d: BW=125kHz, Coding=4/8, Header=explicit
      0xc4, // Reg 0x1e: Spread=4096chips/symbol, CRC=enable
      0x0c  // Reg 0x26: Mobile=On, Agc=On
    };
  rf95.setModemRegisters(&modem_config);

  return true;
}
