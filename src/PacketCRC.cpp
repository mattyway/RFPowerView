#include "PacketCRC.h"

PacketCRC::PacketCRC() : crc(CRC16(0x755B, 0xF48B, 0x0000, false, false)) {
  // Constructor implementation (if needed)
}

PacketCRC::~PacketCRC() {
  // Destructor implementation (if needed)
}

bool PacketCRC::check(const uint8_t* packet) {
  uint8_t length = packet[1];
  
  if (length <= 0 or length > 28) {
    return false;
  }

  uint16_t result = calculate(packet);
  uint8_t checksum1 = (uint8_t)((result & 0xFF00) >> 8);
  uint8_t checksum2 = (uint8_t)(result & 0x00FF);

  if (packet[length + 2] == checksum1 && packet[length + 3] == checksum2) {
    return true;
  }

  return false;
}

uint16_t PacketCRC::calculate(const uint8_t* packet) {
  crc.restart();
  uint8_t length = packet[1];
  crc.add(packet, length + 2);
  return crc.calc();
}