#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include <Arduino.h>
#include "PacketTypes.h"

class PacketParser {
public:
    PacketParser();
    ~PacketParser();

    bool parsePacket(const uint8_t *buffer, Packet& packet);
private:
    bool parseFields(const uint8_t *buffer, std::vector<Field>& fields);
};

#endif // PACKET_PARSER_H