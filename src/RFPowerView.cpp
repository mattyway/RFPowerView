#include "RFPowerView.h"
#include <FunctionalInterrupt.h>

RFPowerView::RFPowerView(uint8_t cePin, uint8_t csPin, uint8_t irqPin, uint16_t rfID) :
    radio(cePin, csPin),
    packetReceiver(&radio),
    bufferFiller(0x3D, 0x96, 0x05),
    irqPin(irqPin),
    rfID{static_cast<uint8_t>(rfID & 0xFF), static_cast<uint8_t>(rfID >> 8)},
    packetCallback(nullptr) {}

bool RFPowerView::begin() {
    if (!radio.begin()) {
        return false;
    }

    packetReceiver.setBufferCallback([this](const uint8_t* buffer) { this->processBuffer(buffer); });
    packetReceiver.setInvalidBufferCallback([this](const uint8_t* buffer) { this->processInvalidBuffer(buffer); });

    pinMode(irqPin, INPUT);

    attachInterrupt(digitalPinToInterrupt(irqPin), std::bind(&RFPowerView::interruptHandler, this), FALLING);

    radio.setChannel(DEFAULT_RF_CHANNEL);
    radio.setDataRate(DEFAULT_RF_DATARATE);
    radio.setAutoAck(false);
    radio.disableCRC();
    radio.setRetries(0, 0);
    radio.setPayloadSize(32);
    radio.setAddressWidth(2);

    startListening();

    return true;
}

void RFPowerView::loop() {
    packetReceiver.loop();
}

void RFPowerView::startListening() {
    radio.openReadingPipe(0, rfID);

    radio.startListening();
    
    interrupts();
}

void RFPowerView::startTransmitting() {
    radio.stopListening();

    radio.setPALevel(RF24_PA_HIGH, true);

    radio.openWritingPipe(rfID);
    radio.powerUp();
}

void RFPowerView::interruptHandler() {
    packetReceiver.read(); 
}

void RFPowerView::processBuffer(const uint8_t *buffer) {
    Packet packet;
    bool result = packetParser.parsePacket(buffer, packet);
    if (result) {
        if (packetCallback != nullptr) {
            packetCallback(&packet);
        }
    }
}

void RFPowerView::processInvalidBuffer(const uint8_t *buffer) {
    
}

void RFPowerView::setPacketCallback(std::function<void(const Packet*)> callback) {
    packetCallback = callback;
}

bool RFPowerView::sendPacket(const Packet* packet) {
    if (bufferFiller.fill(sendBuffer, packet)) {
        startTransmitting();

        uint8_t bytecount = sendBuffer[1] + 4;
        for (int j = 1; j < 5; j++) {
            radio.openWritingPipe(rfID);
            for (int i = 1; i < 200; i++) {
                radio.writeFast(sendBuffer, bytecount);
            }
            delay(100);
            radio.flush_tx();
            radio.txStandBy();
        }

        startListening();

        return true;
    }
    return false;
}