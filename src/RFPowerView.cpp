#include "RFPowerView.h"
#include <FunctionalInterrupt.h>

RFPowerView::RFPowerView(uint8_t cePin, uint8_t csPin, uint8_t irqPin, uint16_t rfID) :
    radio(cePin, csPin),
    packetReceiver(&radio),
    irqPin(irqPin),
    rfID{static_cast<uint8_t>(rfID & 0xFF), static_cast<uint8_t>(rfID >> 8)} {}

bool RFPowerView::begin() {
    if (!radio.begin()) {
        return false;
    }

    packetReceiver.setPacketCallback([this](const uint8_t* buffer) { this->processPacketBuffer(buffer); });
    packetReceiver.setInvalidPacketCallback([this](const uint8_t* buffer) { this->processInvalidPacketBuffer(buffer); });

    startListening();

    return true;
}

void RFPowerView::loop() {
    packetReceiver.loop();
}

void RFPowerView::startListening() {
    radio.setChannel(DEFAULT_RF_CHANNEL);
    radio.setDataRate(DEFAULT_RF_DATARATE);
    radio.setAutoAck(false);
    radio.disableCRC();
    radio.setPayloadSize(32);

    // TODO: Verify that putting this in begin() instead of startListening() works
    pinMode(irqPin, INPUT);

    radio.setAddressWidth(2);
    radio.openReadingPipe(0, rfID);

    radio.startListening();
    Serial.println("Listening");
    // TODO: Verify that putting this in being() instead of startListening() works
    attachInterrupt(digitalPinToInterrupt(irqPin), std::bind(&RFPowerView::interruptHandler, this), FALLING);
    interrupts();
}

void RFPowerView::interruptHandler() {
    packetReceiver.read(); 
}

void RFPowerView::processPacketBuffer(const uint8_t *buffer) {
    Packet packet;
    bool result = packetParser.parsePacket(buffer, packet);
    if (result) {
        Serial.print("Parsed packet! ");
        Serial.print((int)packet.type);
        Serial.println();
    }
}

void RFPowerView::processInvalidPacketBuffer(const uint8_t *buffer) {
    
}