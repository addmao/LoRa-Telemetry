#ifndef __PRINTF_H__
#define __PRINTF_H__

// Adapted from https://playground.arduino.cc/Main/Printf

void printf(Stream& stream, char *fmt, ... );
void printf(Stream& stream, const __FlashStringHelper *fmt, ... );

#endif
