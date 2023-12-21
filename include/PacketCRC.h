#ifndef PACKET_CRC_H
#define PACKET_CRC_H

#include <CRC16.h>

class PacketCRC {
public:
    PacketCRC();
    ~PacketCRC();

    // Method to calculate CRC for a packet
    bool check(const uint8_t* packet);
    uint16_t calculate(const uint8_t* packet);
private:
    CRC16 crc;
};

#endif // PACKET_CRC_H