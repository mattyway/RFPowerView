#ifndef BUFFERFILLER_H
#define BUFFERFILLER_H

#include <Arduino.h>
#include "PacketCRC.h"
#include "PacketTypes.h"

class BufferFiller {
public:
    BufferFiller(uint8_t rollingCode1, uint8_t rollingCode2, uint8_t protocolVersion);
    ~BufferFiller();

    bool fill(uint8_t *buffer, const Packet* packet);

private:
    uint8_t rollingCode1;
    uint8_t rollingCode2;
    uint8_t protocolVersion;

    PacketCRC packetCRC;

    void setPacketSize(uint8_t *buffer, uint8_t);
    void setConstants(uint8_t *buffer);
    void setSourceAddress(uint8_t *buffer, uint16_t);
    void setDestinationAddress(uint8_t *buffer, uint16_t);
    void setRollingCodes(uint8_t *buffer);
    void setProtocolVersion(uint8_t *buffer, uint8_t);
    void setOpenPacketData(uint8_t *buffer);
    void setClosePacketData(uint8_t *buffer);
    void setStopPacketData(uint8_t *buffer);
    void calculateCRC(uint8_t *buffer);
    void incrementRollingCodes();
};

#endif // BUFFERFILLER_H