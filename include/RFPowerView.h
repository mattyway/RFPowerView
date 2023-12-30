#ifndef RFPOWERVIEW_H
#define RFPOWERVIEW_H

#include <Arduino.h>
#include <RF24.h>
#include "PacketReceiver.h"
#include "PacketParser.h"
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
    void startListening();

    void setPacketCallback(std::function<void(const Packet*)> callback);

private:
    RF24 radio;
    PacketReceiver packetReceiver;
    PacketParser packetParser;
    uint8_t irqPin;
    uint8_t rfID[2];

    std::function<void(const Packet*)> packetCallback;

    void interruptHandler();

    void processBuffer(const uint8_t *buffer);
    void processInvalidBuffer(const uint8_t *buffer);
};

#endif // RFPOWERVIEW_H