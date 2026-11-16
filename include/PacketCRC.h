#ifndef PACKET_CRC_H
#define PACKET_CRC_H

#include <CRC16.h>

class PacketCRC {
public:
    explicit PacketCRC(uint16_t rfID);
    ~PacketCRC();

    // Method to calculate CRC for a packet
    bool check(const uint8_t* packet);
    uint16_t calculate(const uint8_t* packet);
private:
    uint16_t rfID;
    CRC16 crc;
};

#endif // PACKET_CRC_H