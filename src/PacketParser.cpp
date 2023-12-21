#include "PacketParser.h"

PacketParser::PacketParser()
{
}

PacketParser::~PacketParser()
{
}

bool PacketParser::parsePacket(const uint8_t *buffer, Packet& message)
{
    message.source = (uint16_t)(buffer[14] << 8 | buffer[15]);
    message.destination = (uint16_t)(buffer[12] << 8 | buffer[13]);

    if (buffer[16] == 0x52 && buffer[17] == 0x53) {
        message.type = PacketType::STOP;
        message.parameters = std::monostate{};
    } else if (buffer[16] == 0x52 && buffer[17] == 0x44) {
        message.type = PacketType::CLOSE;
        message.parameters = std::monostate{};
    } else if (buffer[16] == 0x52 && buffer[17] == 0x55) {
        message.type = PacketType::OPEN;
        message.parameters = std::monostate{};
    } else if (buffer[16] == 0x21 && buffer[17] == 0x5A) { 
        message.type = PacketType::FIELDS;
        std::vector<Field> fields;
        parseFields(buffer, fields);
        message.parameters = FieldsParameters{fields};
    } else if (buffer[16] == 0x3F && buffer[17] == 0x5A) {
        message.type = PacketType::FIELD_COMMAND;
        std::vector<Field> fields;
        parseFields(buffer, fields);
        message.parameters = FieldsParameters{fields};
    } else {
        message.type = PacketType::UNKNOWN;
        message.parameters = std::monostate{};
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