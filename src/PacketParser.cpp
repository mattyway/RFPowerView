#include "PacketParser.h"
#include <cstddef>

PacketParser::PacketParser()
{
}

PacketParser::~PacketParser()
{
}

bool PacketParser::parsePacket(const uint8_t *buffer, Packet& packet)
{
    int dataOffset = -1;
    if (buffer[10] == 0x04) {
        BroadcastHeader header{};
        header.source = (uint16_t)(buffer[12] << 8 | buffer[13]);
        packet.header = header;
        dataOffset = 14;
    } else if(buffer[10] == 0x05) {
        UnicastHeader header{};
        header.destination = (uint16_t)(buffer[12] << 8 | buffer[13]);
        header.source = (uint16_t)(buffer[14] << 8 | buffer[15]);
        packet.header = header;
        dataOffset = 16;
    } else if (buffer[10] == 0x06) {
        GroupsHeader header{};
        size_t i = 12;
        uint8_t length = buffer[1] + 2;
        while (i < length) {
            if (buffer[i] == 0x00) {
                dataOffset = i + 3;
                break;
            } else {
                header.groups.push_back(buffer[i]);
            }
            i++;
        }
        if (i == length) {
            return false;
        }
        header.source = (uint16_t)(buffer[i + 1] << 8 | buffer[i + 2]);
        packet.header = header;
    }

    packet.rollingCode1 = buffer[4];
    packet.rollingCode2 = buffer[11];

    if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x53) {
        packet.type = PacketType::STOP;
        packet.parameters = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x44) {
        packet.type = PacketType::CLOSE;
        packet.parameters = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x55) {
        packet.type = PacketType::OPEN;
        packet.parameters = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x4C) {
        packet.type = PacketType::CLOSE_SLOW;
        packet.parameters = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x52) {
        packet.type = PacketType::OPEN_SLOW;
        packet.parameters = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x48) {
        packet.type = PacketType::MOVE_TO_SAVED_POSITION;
        packet.parameters = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x21 && buffer[dataOffset + 1] == 0x5A) { 
        packet.type = PacketType::FIELDS;
        std::vector<Field> fields;
        parseFields(buffer, fields);
        packet.parameters = FieldsParameters{fields};
    } else if (buffer[dataOffset + 0] == 0x3F && buffer[dataOffset + 1] == 0x5A) {
        packet.type = PacketType::FIELD_COMMAND;
        std::vector<Field> fields;
        parseFields(buffer, fields);
        packet.parameters = FieldsParameters{fields};
    } else {
        packet.type = PacketType::UNKNOWN;
        packet.parameters = std::monostate{};
        return false;
    }

    return true;
}

bool PacketParser::parseFields(const uint8_t *buffer, std::vector<Field>& fields) {
    uint8_t length = buffer[1] + 2;

    uint8_t offset = 18;
    while (offset < length) {
        uint8_t fieldLength = buffer[offset];
        if (offset + fieldLength >= length) {
            // Not enough space to parse the field
            return false;
        }
        FieldType type;
        switch (buffer[offset + 1]) {
            case 0x40:
                type = FieldType::SET;
                break;
            case 0x3F:
                type = FieldType::FETCH;
                break;
            case 0x21:
                type = FieldType::VALUE;
                break;
            default:
                type = FieldType::UNKNOWN;
                break;
        } 
        uint8_t identifier = buffer[offset + 2];
        if (fieldLength == 2) {
            fields.push_back(Field{identifier, type, false, std::monostate{}});
        } else if (fieldLength == 3) {
            uint8_t value = buffer[offset + 3];
            fields.push_back(Field{identifier, type, true, value});
        } else if (fieldLength == 4) {
            uint16_t value = (uint16_t)(buffer[offset + 3] | buffer[offset + 4] << 8);
            fields.push_back(Field{identifier, type, true, value});
        }
        offset += fieldLength + 1;
    }
    return true;
}