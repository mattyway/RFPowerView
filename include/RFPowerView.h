#ifndef RFPOWERVIEW_H
#define RFPOWERVIEW_H

#include <stdint.h>
#include <RF24.h>
#include "PacketReceiver.h"
#include "PacketParser.h"
#include "BufferFiller.h"
#include "PacketTypes.h"

#define DEFAULT_RF_CHANNEL             (7) //this is the channel HD shades use
#define DEFAULT_RF_DATARATE            (RF24_1MBPS) //this is the speed HD shades use

class RFPowerView {
public:
    RFPowerView(
        uint8_t cePin, 
        uint8_t csPin,
        uint8_t irqPin,
        uint16_t rfID
    );

    bool begin();
    void loop();

    void setPacketReceivedCallback(std::function<void(const Packet*)> callback);
    void setValidBufferReceivedCallback(std::function<void(const uint8_t*)> callback);
    void setInvalidBufferReceivedCallback(std::function<void(const uint8_t*)> callback);

    bool sendPacket(const Packet* packet);

private:
    RF24 radio;
    PacketReceiver packetReceiver;
    PacketParser packetParser;
    BufferFiller bufferFiller;
    uint8_t irqPin;
    uint8_t rfID[2];

    std::function<void(const Packet*)> packetReceivedCallback;
    std::function<void(const uint8_t*)> validBufferReceivedCallback;
    std::function<void(const uint8_t*)> invalidBufferReceivedCallback;

    uint8_t sendBuffer[32];

    void startListening();
    void startTransmitting();

    void interruptHandler();

    void processBuffer(const uint8_t *buffer);
    void processInvalidBuffer(const uint8_t *buffer);
};

#endif // RFPOWERVIEW_H