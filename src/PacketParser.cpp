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
    packet.senderType = (buffer[2] == 0x10) ? SenderType::SHADE : SenderType::CONTROLLER;

    packet.physicalSource = (uint16_t)(buffer[7] << 8 | buffer[8]);
    packet.repeated = (buffer[9] == 0x85);

    if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x53) {
        packet.type = PacketType::STOP;
        packet.payload = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x44) {
        packet.type = PacketType::CLOSE;
        packet.payload = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x55) {
        packet.type = PacketType::OPEN;
        packet.payload = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x4C) {
        packet.type = PacketType::CLOSE_SLOW;
        packet.payload = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x52) {
        packet.type = PacketType::OPEN_SLOW;
        packet.payload = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x52 && buffer[dataOffset + 1] == 0x48) {
        packet.type = PacketType::MOVE_TO_SAVED_POSITION;
        packet.payload = std::monostate{};
    } else if (buffer[dataOffset + 0] == 0x3F && buffer[dataOffset + 1] == 0x46) {
        packet.type = PacketType::GET_FIRMWARE;
        FirmwarePayload fw{};
        fw.target = static_cast<FirmwareTarget>(buffer[dataOffset + 2]);
        packet.payload = fw;
    } else if (buffer[dataOffset + 0] == 0x21 && buffer[dataOffset + 1] == 0x46) {
        packet.type = PacketType::REPORT_FIRMWARE;
        FirmwarePayload fw{};
        fw.target = static_cast<FirmwareTarget>(buffer[dataOffset + 2]);
        if (buffer[1] >= (dataOffset - 2 + 7)) { // Ensure at least 4 payload data bytes
            uint8_t b0 = buffer[dataOffset + 3];
            uint8_t b1 = buffer[dataOffset + 4];
            uint8_t b2 = buffer[dataOffset + 5];
            uint8_t b3 = buffer[dataOffset + 6];
            fw.build = (uint16_t)(b1 << 8 | b0);
            fw.minor = b2;
            fw.major = b3;
        }
        packet.payload = fw;
    } else if (buffer[dataOffset + 0] == 0x21 && buffer[dataOffset + 1] == 0x5A) { 
        packet.type = PacketType::REPORT_PROPERTIES;
        std::vector<Property> properties;
        parseProperties(buffer, dataOffset + 2, properties);
        packet.payload = PropertiesPayload{properties};
    } else if (buffer[dataOffset + 0] == 0x3F && buffer[dataOffset + 1] == 0x5A) {
        std::vector<Property> properties;
        parseProperties(buffer, dataOffset + 2, properties);
        bool hasSet = false;
        for (const auto& p : properties) {
            if (p.type == PropertyType::SET) {
                hasSet = true;
                break;
            }
        }
        packet.type = hasSet ? PacketType::SET_PROPERTIES : PacketType::GET_PROPERTIES;
        packet.payload = PropertiesPayload{properties};
    } else if (buffer[dataOffset + 0] == 0x53) {
        if (buffer[dataOffset + 1] == 0x47) {
            packet.type = PacketType::ACTIVATE_SCENE;
        } else if (buffer[dataOffset + 1] == 0x43) {
            packet.type = PacketType::STORE_SCENE;
        } else {
            packet.type = PacketType::UNKNOWN;
            packet.payload = std::monostate{};
            return false;
        }
        packet.payload = ScenePayload{buffer[dataOffset + 2]};
    } else {
        packet.type = PacketType::UNKNOWN;
        packet.payload = std::monostate{};
        return false;
    }

    return true;
}

bool PacketParser::parseProperties(const uint8_t *buffer, uint8_t offset, std::vector<Property>& properties) {
    uint8_t length = buffer[1] + 2;

    while (offset < length) {
        uint8_t propertyLength = buffer[offset];
        if (offset + propertyLength >= length) {
            // Not enough space to parse the property
            return false;
        }
        PropertyType type;
        switch (buffer[offset + 1]) {
            case 0x40:
                type = PropertyType::SET;
                break;
            case 0x3F:
                type = PropertyType::FETCH;
                break;
            case 0x21:
                type = PropertyType::VALUE;
                break;
            default:
                type = PropertyType::UNKNOWN;
                break;
        } 
        uint8_t identifier = buffer[offset + 2];
        if (propertyLength == 2) {
            properties.push_back(Property{identifier, type, false, std::monostate{}});
        } else if (propertyLength == 3) {
            uint8_t value = buffer[offset + 3];
            properties.push_back(Property{identifier, type, true, value});
        } else if (propertyLength == 4) {
            uint16_t value = (uint16_t)(buffer[offset + 3] | buffer[offset + 4] << 8);
            properties.push_back(Property{identifier, type, true, value});
        }
        offset += propertyLength + 1;
    }
    return true;
}