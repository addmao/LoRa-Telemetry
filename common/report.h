#ifndef __REPORT_H__
#define __REPORT_H__

struct SensorData {
  uint16_t checkingField;
  uint16_t level;
  uint16_t air_temp;
  uint16_t humidity;
  uint16_t water_temp;
  uint16_t voltage;
} __attribute__((packed));

#endif
