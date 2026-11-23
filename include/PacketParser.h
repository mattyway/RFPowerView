#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include <vector>
#include "PacketTypes.h"

class PacketParser {
public:
    PacketParser();
    ~PacketParser();

    bool parsePacket(const uint8_t *buffer, Packet& packet);
private:
    bool parseProperties(const uint8_t *buffer, uint8_t offset, std::vector<Property>& properties);
};

#endif // PACKET_PARSER_H