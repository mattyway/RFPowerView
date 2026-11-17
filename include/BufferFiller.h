#ifndef BUFFERFILLER_H
#define BUFFERFILLER_H

#include <stdint.h>
#include "PacketCRC.h"
#include "PacketTypes.h"

class BufferFiller {
public:
    explicit BufferFiller(uint16_t rfID);
    ~BufferFiller();

    bool fill(uint8_t *buffer, const Packet* packet);

private:
    PacketCRC packetCRC;

    void setPacketSize(uint8_t *buffer, uint8_t size);
    void setPacketSize(uint8_t *buffer, uint8_t dataOffset, uint8_t dataLength);
    void setConstants(uint8_t *buffer);
    void setSenderType(uint8_t *buffer, SenderType senderType);
    void setSourceAddress(uint8_t *buffer, uint8_t offset, uint16_t source, uint16_t physicalSource = 0);
    void setDestinationAddress(uint8_t *buffer, uint8_t offset, uint16_t destination);
    void setRepeatFlag(uint8_t *buffer, bool repeated);
    void setRollingCodes(uint8_t *buffer, uint8_t rollingCode1, uint8_t rollingCode2);
    void setProtocolVersion(uint8_t *buffer, uint8_t);
    void setFieldsData(uint8_t *buffer, uint8_t offset, const FieldsParameters& parameters);
    void calculateCRC(uint8_t *buffer);
    void incrementRollingCodes();
    uint8_t calculateTotalFieldSize(const FieldsParameters& parameters);
    uint8_t calculateFieldSize(const Field& field);
};

#endif // BUFFERFILLER_H