#ifndef __PACKETS_H__
#define __PACKETS_H__

#define PACKET_TYPE_CONFIG  0x1201
#define PACKET_TYPE_DATA    0x1202

struct PacketHeader {
  uint16_t type;
} __attribute__((packed));

struct PacketConfig {
  PacketHeader hdr;
  uint16_t     interval;
} __attribute__((packed));

struct PacketData {
  PacketHeader hdr;
  uint16_t level;
  uint16_t air_temp;
  uint16_t humidity;
  uint16_t water_temp;
  uint16_t voltage;
} __attribute__((packed));

#endif
