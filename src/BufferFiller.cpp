#include "BufferFiller.h"

BufferFiller::BufferFiller(uint8_t rollingCode1, uint8_t rollingCode2, uint8_t protocolVersion) : 
    rollingCode1(rollingCode1),
    rollingCode2(rollingCode2),
    protocolVersion(protocolVersion)
{
}

BufferFiller::~BufferFiller()
{
}

bool BufferFiller::fill(uint8_t *buffer, const Packet* packet) {
  uint8_t packetSize = 0;
  switch(packet->type) {
    case PacketType::STOP:
    case PacketType::OPEN:
    case PacketType::CLOSE:
      packetSize = 0x11;
      break;
    // TODO: Calculate fields length in buffer
    default:
      return false;
  }

  setPacketSize(buffer, packetSize);
  setConstants(buffer);
  setProtocolVersion(buffer, protocolVersion);
  setSourceAddress(buffer, packet->source);
  setDestinationAddress(buffer, packet->destination);

  switch(packet->type) {
    case PacketType::STOP:
      setStopPacketData(buffer);
      break;
    case PacketType::CLOSE:
      setClosePacketData(buffer);
      break;
    case PacketType::OPEN:
      setOpenPacketData(buffer);
      break;
    default:
      return false;
  }

  setRollingCodes(buffer);
  calculateCRC(buffer);

  incrementRollingCodes();

  return true;
}

void BufferFiller::setOpenPacketData(uint8_t *buffer)
{
    buffer[16] = 0x52;
    buffer[17] = 0x55;
    buffer[18] = 0x00;
}

void BufferFiller::setClosePacketData(uint8_t *buffer)
{
    buffer[16] = 0x52;
    buffer[17] = 0x44;
    buffer[18] = 0x00;
}


void BufferFiller::setStopPacketData(uint8_t *buffer)
{
    buffer[16] = 0x52;
    buffer[17] = 0x53;
    buffer[18] = 0x00;
}

/*
void BufferFiller::setPositionCommand(uint8_t *buffer, float percentage)
{
    // packetSize = 0x15;

    buffer[16] = 0x3F; // Field command
    buffer[17] = 0x5A; // Constant

    buffer[18] = 0x04; // Field of 4 bytes
    buffer[19] = 0x40; // Field type (0x40 is set)
    buffer[20] = 0x50; // ID of position field

    uint16_t position = (uint16_t)(0xFFFF * percentage);

    buffer[21] = (uint8_t)(position & 0x00FF);
    buffer[22] = (uint8_t)((position & 0xFF00) >> 8);
}

 void BufferFiller::setFetchPositionCommand(uint8_t *buffer) {
    // packetSize = 0x13;
    
    buffer[16] = 0x3F; // Field command
    buffer[17] = 0x5A; // Constant

    buffer[18] = 0x02; // Field of 2 bytes
    buffer[19] = 0x3F; // Field type (0x3F is fetch)
    buffer[20] = 0x50; // ID of position field
 }
*/

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

void BufferFiller::setRollingCodes(uint8_t *buffer) {
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

void BufferFiller::incrementRollingCodes() {
  rollingCode1++;
  rollingCode2++;
}