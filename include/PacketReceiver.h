#ifndef PACKET_RECEIVER_H
#define PACKET_RECEIVER_H

#define CIRCULAR_BUFFER_INT_SAFE

#include <functional>
#include <RF24.h>
#include <CircularBuffer.h>
#include "PacketCRC.h"

#define BUFFER_SIZE 32

#define TOTAL_BUFFER_COUNT 30
#define RX_BUFFER_COUNT 3
#define EMPTY_BUFFER_COUNT 30
#define VALID_BUFFER_COUNT 5

class PacketReceiver {
public:
    PacketReceiver(RF24 *radio);
    ~PacketReceiver();

    void loop();
    void read();

    void setPacketCallback(std::function<void(const uint8_t*)> callback);
    void setInvalidPacketCallback(std::function<void(const uint8_t*)> callback);

private:
    RF24 *radio;
    PacketCRC packetCRC;

    uint8_t buffers[TOTAL_BUFFER_COUNT][BUFFER_SIZE];
    CircularBuffer<uint8_t*, RX_BUFFER_COUNT> pendingBuffers;
    CircularBuffer<uint8_t*, EMPTY_BUFFER_COUNT> freeBuffers;
    CircularBuffer<uint8_t*, VALID_BUFFER_COUNT> receivedBuffers;

    std::function<void(const uint8_t*)> packetCallback;
    std::function<void(const uint8_t*)> invalidPacketCallback;

    bool isSanePacket(uint8_t *buffer);
    bool isValidPacket(uint8_t *buffer);
    bool isNewPacket(uint8_t *buffer);
    bool isEquivalentPacket(uint8_t *a, uint8_t *b);
};

#endif // PACKET_RECEIVER_H