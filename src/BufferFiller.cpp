#include "BufferFiller.h"

BufferFiller::BufferFiller(uint8_t protocolVersion) : 
    protocolVersion(protocolVersion)
{
}

BufferFiller::~BufferFiller()
{
}

bool BufferFiller::fill(uint8_t *buffer, const Packet* packet) {
  setConstants(buffer);
  setProtocolVersion(buffer, protocolVersion);
  setSourceAddress(buffer, packet->source);
  setDestinationAddress(buffer, packet->destination);

  switch(packet->type) {
    case PacketType::STOP:
      setPacketSize(buffer, 0x11);
      buffer[16] = 0x52;
      buffer[17] = 0x53;
      buffer[18] = 0x00;
      break;
    case PacketType::CLOSE:
      setPacketSize(buffer, 0x11);
      buffer[16] = 0x52;
      buffer[17] = 0x44;
      buffer[18] = 0x00;
      break;
    case PacketType::OPEN:
      setPacketSize(buffer, 0x11);
      buffer[16] = 0x52;
      buffer[17] = 0x55;
      buffer[18] = 0x00;
      break;
    case PacketType::FIELDS: {
      FieldsParameters parameters = std::get<FieldsParameters>(packet->parameters);
      // 0x10 is the number of bytes without any fields
      setPacketSize(buffer, 0x10 + calculateTotalFieldSize(parameters));
      buffer[16] = 0x21;
      buffer[17] = 0x5A;
      setFieldsData(buffer, parameters);
      break;
    }
    case PacketType::FIELD_COMMAND: {
      FieldsParameters parameters = std::get<FieldsParameters>(packet->parameters);
      // 0x10 is the number of bytes without any fields
      setPacketSize(buffer, 0x10 + calculateTotalFieldSize(parameters));
      buffer[16] = 0x3F;
      buffer[17] = 0x5A;
      setFieldsData(buffer, std::get<FieldsParameters>(packet->parameters));
      break;
    }
    default:
      return false;
  }

  setRollingCodes(buffer, packet->rollingCode1, packet->rollingCode2);
  calculateCRC(buffer);

  return true;
}

void BufferFiller::setFieldsData(uint8_t *buffer, const FieldsParameters& parameters) {
  uint8_t offset = 18;

  for (size_t i = 0; i < parameters.fields.size(); i++) {
    Field field = parameters.fields[i];
    uint8_t fieldSize = calculateFieldSize(field);
    buffer[offset] = fieldSize - 1; // Minus 1 because the length byte isn't counted
    switch(field.type) {
      case FieldType::SET:
        buffer[offset + 1] = 0x40;
        break;
      case FieldType::FETCH:
        buffer[offset + 1] = 0x3F;
        break;
      case FieldType::VALUE:
        buffer[offset + 1] = 0x21;
        break;
      default:
        // TODO: Return false?
        break;
    }
    buffer[offset + 2] = field.identifier;
    if(field.hasValue) {
      if (std::holds_alternative<uint8_t>(field.value)) {
        buffer[offset + 3] = std::get<uint8_t>(field.value);
      } else if (std::holds_alternative<uint16_t>(field.value)) {
        uint16_t value = std::get<uint16_t>(field.value);
        buffer[offset + 3] = (uint8_t)(value & 0x00FF);
        buffer[offset + 4] = (uint8_t)((value & 0xFF00) >> 8);
      }
    }
    offset += fieldSize;
  }
}

void BufferFiller::setPacketSize(uint8_t *buffer, uint8_t length) {
  buffer[1] = length;  // Packet size
}

void BufferFiller::setConstants(uint8_t *buffer) {
  buffer[0] = 0xC0;  // Header byte

  buffer[2] = 0x00;  // Constant when sending, can be 0x10 when receiving a packet
  buffer[3] = 0x05;  // Constant

  buffer[5] = 0xFF;  // Constant
  buffer[6] = 0xFF;  // Constant

  buffer[9] = 0x86;  // Constant?
}

void BufferFiller::setSourceAddress(uint8_t *buffer, uint16_t sourceID) {
  // Physical source address (could be the address of a repeater when receiving a packet)
  buffer[7] = (uint8_t)((sourceID & 0xFF00) >> 8);
  buffer[8] = (uint8_t)(sourceID & 0x00FF);

  // Logical source address (usually the same as the physical source address)
  buffer[14] = (uint8_t)((sourceID & 0xFF00) >> 8);
  buffer[15] = (uint8_t)(sourceID & 0x00FF);
}

void BufferFiller::setDestinationAddress(uint8_t *buffer, uint16_t targetID) {
  // Logical target address
  buffer[12] = (uint8_t)((targetID & 0xFF00) >> 8);
  buffer[13] = (uint8_t)(targetID & 0x00FF);
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

uint8_t BufferFiller::calculateTotalFieldSize(const FieldsParameters& parameters) {
  uint8_t totalSize = 0;
  for (size_t i = 0; i < parameters.fields.size(); i++) {
    Field field = parameters.fields[i];
    totalSize += calculateFieldSize(field);
  }
  return totalSize;
}

uint8_t BufferFiller::calculateFieldSize(const Field& field) {
  uint8_t fieldSize = 3; // Length byte + field type byte + identifier byte
  if (field.hasValue) {
    if (std::holds_alternative<uint8_t>(field.value)) {
      fieldSize += 1;
    } else if (std::holds_alternative<uint16_t>(field.value)) {
      fieldSize += 2;
    }
  }
  return fieldSize;
}