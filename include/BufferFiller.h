#ifndef BUFFERFILLER_H
#define BUFFERFILLER_H

#include <stdint.h>
#include "PacketCRC.h"
#include "PacketTypes.h"

class BufferFiller {
public:
    BufferFiller(uint8_t protocolVersion);
    ~BufferFiller();

    bool fill(uint8_t *buffer, const Packet* packet);

private:
    uint8_t protocolVersion;

    PacketCRC packetCRC;

    void setPacketSize(uint8_t *buffer, uint8_t);
    void setConstants(uint8_t *buffer);
    void setSourceAddress(uint8_t *buffer, uint8_t offset, uint16_t source);
    void setDestinationAddress(uint8_t *buffer, uint8_t offset, uint16_t destination);
    void setRollingCodes(uint8_t *buffer, uint8_t rollingCode1, uint8_t rollingCode2);
    void setProtocolVersion(uint8_t *buffer, uint8_t);
    void setFieldsData(uint8_t *buffer, uint8_t offset, const FieldsParameters& parameters);
    void calculateCRC(uint8_t *buffer);
    void incrementRollingCodes();
    uint8_t calculateTotalFieldSize(const FieldsParameters& parameters);
    uint8_t calculateFieldSize(const Field& field);
};

#endif // BUFFERFILLER_H