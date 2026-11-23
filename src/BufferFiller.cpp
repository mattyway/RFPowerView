#include "BufferFiller.h"

BufferFiller::BufferFiller(uint16_t rfID) :
    packetCRC(rfID)
{
}

BufferFiller::~BufferFiller()
{
}

bool BufferFiller::fill(uint8_t *buffer, const Packet* packet) {
  setConstants(buffer);
  setSenderType(buffer, packet->senderType);

  int dataOffset = -1;

  if (std::holds_alternative<BroadcastHeader>(packet->header)) {
    setProtocolVersion(buffer, 0x04);
    auto header = std::get<BroadcastHeader>(packet->header);
    setSourceAddress(buffer, 12, header.source, packet->physicalSource);
    dataOffset = 14;
  } else if (std::holds_alternative<UnicastHeader>(packet->header)) {
    setProtocolVersion(buffer, 0x05);
    auto header = std::get<UnicastHeader>(packet->header);
    setDestinationAddress(buffer, 12, header.destination);
    setSourceAddress(buffer, 14, header.source, packet->physicalSource);
    dataOffset = 16;
  } else if (std::holds_alternative<GroupsHeader>(packet->header)) {
    setProtocolVersion(buffer, 0x06);
    auto header = std::get<GroupsHeader>(packet->header);
    int groupsOffset = 12;
    for (size_t i = 0; i < header.groups.size(); i++) {
      buffer[groupsOffset] = header.groups[i];
      groupsOffset++;
    }
    buffer[groupsOffset] = 0x00;
    setSourceAddress(buffer, groupsOffset + 1, header.source, packet->physicalSource);
    dataOffset = groupsOffset + 3;
  }

  switch(packet->type) {
    case PacketType::STOP:
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x52;
      buffer[dataOffset + 1] = 0x53;
      buffer[dataOffset + 2] = 0x00;
      break;
    case PacketType::CLOSE:
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x52;
      buffer[dataOffset + 1] = 0x44;
      buffer[dataOffset + 2] = 0x00;
      break;
    case PacketType::OPEN:
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x52;
      buffer[dataOffset + 1] = 0x55;
      buffer[dataOffset + 2] = 0x00;
      break;
    case PacketType::CLOSE_SLOW:
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x52;
      buffer[dataOffset + 1] = 0x4C;
      buffer[dataOffset + 2] = 0x00;
      break;
    case PacketType::OPEN_SLOW:
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x52;
      buffer[dataOffset + 1] = 0x52;
      buffer[dataOffset + 2] = 0x00;
      break;
    case PacketType::MOVE_TO_SAVED_POSITION:
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x52;
      buffer[dataOffset + 1] = 0x48;
      buffer[dataOffset + 2] = 0x00;
      break;
    case PacketType::REPORT_PROPERTIES: {
      if (!std::holds_alternative<PropertiesPayload>(packet->payload)) {
        return false;
      }
      PropertiesPayload payload = std::get<PropertiesPayload>(packet->payload);
      setPacketSize(buffer, dataOffset, calculateTotalPropertiesSize(payload));
      buffer[dataOffset + 0] = 0x21;
      buffer[dataOffset + 1] = 0x5A;
      setPropertiesData(buffer, dataOffset + 2, payload);
      break;
    }
    case PacketType::GET_PROPERTIES:
    case PacketType::SET_PROPERTIES: {
      if (!std::holds_alternative<PropertiesPayload>(packet->payload)) {
        return false;
      }
      PropertiesPayload payload = std::get<PropertiesPayload>(packet->payload);
      setPacketSize(buffer, dataOffset, calculateTotalPropertiesSize(payload));
      buffer[dataOffset + 0] = 0x3F;
      buffer[dataOffset + 1] = 0x5A;
      setPropertiesData(buffer, dataOffset + 2, payload);
      break;
    }
    case PacketType::ACTIVATE_SCENE:
    case PacketType::STORE_SCENE: {
      if (!std::holds_alternative<ScenePayload>(packet->payload)) {
        return false;
      }
      ScenePayload payload = std::get<ScenePayload>(packet->payload);
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x53;
      buffer[dataOffset + 1] = (packet->type == PacketType::ACTIVATE_SCENE) ? 0x47 : 0x43;
      buffer[dataOffset + 2] = payload.sceneID;
      break;
    }
    case PacketType::GET_FIRMWARE: {
      if (!std::holds_alternative<FirmwarePayload>(packet->payload)) {
        return false;
      }
      FirmwarePayload payload = std::get<FirmwarePayload>(packet->payload);
      setPacketSize(buffer, dataOffset, 3);
      buffer[dataOffset + 0] = 0x3F;
      buffer[dataOffset + 1] = 0x46;
      buffer[dataOffset + 2] = static_cast<uint8_t>(payload.target);
      break;
    }
    case PacketType::REPORT_FIRMWARE: {
      if (!std::holds_alternative<FirmwarePayload>(packet->payload)) {
        return false;
      }
      FirmwarePayload payload = std::get<FirmwarePayload>(packet->payload);
      setPacketSize(buffer, dataOffset, 7);
      buffer[dataOffset + 0] = 0x21;
      buffer[dataOffset + 1] = 0x46;
      buffer[dataOffset + 2] = static_cast<uint8_t>(payload.target);
      buffer[dataOffset + 3] = (uint8_t)(payload.build & 0x00FF);
      buffer[dataOffset + 4] = (uint8_t)((payload.build & 0xFF00) >> 8);
      buffer[dataOffset + 5] = payload.minor;
      buffer[dataOffset + 6] = payload.major;
      break;
    }
    default:
      return false;
  }

  setRepeatFlag(buffer, packet->repeated);
  setRollingCodes(buffer, packet->rollingCode1, packet->rollingCode2);
  calculateCRC(buffer);

  return true;
}

void BufferFiller::setPropertiesData(uint8_t *buffer, uint8_t offset, const PropertiesPayload& payload) {
  for (size_t i = 0; i < payload.properties.size(); i++) {
    Property property = payload.properties[i];
    uint8_t propertySize = calculatePropertySize(property);
    buffer[offset] = propertySize - 1; // Minus 1 because the length byte isn't counted
    switch(property.type) {
      case PropertyType::SET:
        buffer[offset + 1] = 0x40;
        break;
      case PropertyType::FETCH:
        buffer[offset + 1] = 0x3F;
        break;
      case PropertyType::VALUE:
        buffer[offset + 1] = 0x21;
        break;
      default:
        // TODO: Return false?
        break;
    }
    buffer[offset + 2] = property.identifier;
    if(property.hasValue) {
      if (std::holds_alternative<uint8_t>(property.value)) {
        buffer[offset + 3] = std::get<uint8_t>(property.value);
      } else if (std::holds_alternative<uint16_t>(property.value)) {
        uint16_t value = std::get<uint16_t>(property.value);
        buffer[offset + 3] = (uint8_t)(value & 0x00FF);
        buffer[offset + 4] = (uint8_t)((value & 0xFF00) >> 8);
      }
    }
    offset += propertySize;
  }
}

void BufferFiller::setPacketSize(uint8_t *buffer, uint8_t size) {
  buffer[1] = size;  // Packet size
}

void BufferFiller::setPacketSize(uint8_t *buffer, uint8_t dataOffset, uint8_t dataLength) {
  buffer[1] = (dataOffset - 2) + dataLength;  // Packet size
}

void BufferFiller::setConstants(uint8_t *buffer) {
  buffer[0] = 0xC0;  // Header byte
  buffer[3] = 0x05;  // Constant
  buffer[5] = 0xFF;  // Constant
  buffer[6] = 0xFF;  // Constant
}

void BufferFiller::setSenderType(uint8_t *buffer, SenderType senderType) {
  buffer[2] = static_cast<uint8_t>(senderType);
}

void BufferFiller::setSourceAddress(uint8_t *buffer, uint8_t offset, uint16_t sourceID, uint16_t physicalSource) {
  // Physical source address (could be the address of a repeater)
  uint16_t physicalID = (physicalSource != 0) ? physicalSource : sourceID;
  buffer[7] = (uint8_t)((physicalID & 0xFF00) >> 8);
  buffer[8] = (uint8_t)(physicalID & 0x00FF);

  // Logical source address (usually the same as the physical source address)
  buffer[offset + 0] = (uint8_t)((sourceID & 0xFF00) >> 8);
  buffer[offset + 1] = (uint8_t)(sourceID & 0x00FF);
}

void BufferFiller::setRepeatFlag(uint8_t *buffer, bool repeated) {
  buffer[9] = repeated ? 0x85 : 0x86;
}

void BufferFiller::setDestinationAddress(uint8_t *buffer, uint8_t offset, uint16_t targetID) {
  // Logical target address
  buffer[offset + 0] = (uint8_t)((targetID & 0xFF00) >> 8);
  buffer[offset + 1] = (uint8_t)(targetID & 0x00FF);
}

void BufferFiller::setRollingCodes(uint8_t *buffer, uint8_t rollingCode1, uint8_t rollingCode2) {
  buffer[4] = rollingCode1;  // Rolling code 1
  buffer[11] = rollingCode2; // Rolling code 2
}

void BufferFiller::setProtocolVersion(uint8_t *buffer, uint8_t version) {
  buffer[10] = version; // Protocol version?
}

void BufferFiller::calculateCRC(uint8_t *buffer) { // must be called after the buffer has been filled
  uint8_t length = buffer[1];
  uint16_t result = packetCRC.calculate(buffer);

  uint8_t checksum1 = (uint8_t)((result & 0xFF00) >> 8);
  uint8_t checksum2 = (uint8_t)(result & 0x00FF);

  buffer[length + 2] = checksum1; // Checksum
  buffer[length + 3] = checksum2; // Checksum
}

uint8_t BufferFiller::calculateTotalPropertiesSize(const PropertiesPayload& payload) {
  uint8_t totalSize = 2;
  for (size_t i = 0; i < payload.properties.size(); i++) {
    Property property = payload.properties[i];
    totalSize += calculatePropertySize(property);
  }
  return totalSize;
}

uint8_t BufferFiller::calculatePropertySize(const Property& property) {
  uint8_t propertySize = 3; // Length byte + property type byte + identifier byte
  if (property.hasValue) {
    if (std::holds_alternative<uint8_t>(property.value)) {
      propertySize += 1;
    } else if (std::holds_alternative<uint16_t>(property.value)) {
      propertySize += 2;
    }
  }
  return propertySize;
}