#include "PacketReceiver.h"

PacketReceiver::PacketReceiver(RF24 *radio) : radio(radio) {
    for (int i = 0; i < TOTAL_BUFFER_COUNT; i++) {
        if (!freeBuffers.isFull()) {
            freeBuffers.push(buffers[i]);
        }
    }
}

PacketReceiver::~PacketReceiver() {

}

void PacketReceiver::loop() {
    noInterrupts();

    while (!pendingBuffers.isEmpty()) {
        uint8_t *p = pendingBuffers.pop();

        bool isSaneBuffer = this->isSaneBuffer(p);
        bool isValidBuffer = this->isValidBuffer(p);
        bool isNewBuffer = this->isNewBuffer(p);

        // TODO: We don't keep old invalid packets around, so invalid packets will always be new for now
        if (isSaneBuffer && !isValidBuffer && isNewBuffer) {
          if (invalidBufferCallback != nullptr) {
            invalidBufferCallback(p);
          }
        }

        if (isValidBuffer && isNewBuffer) {
          if (bufferCallback != nullptr) {
            this->bufferCallback(p);
          }

            // If there is no space to store this new packet, return the oldest packet
            if (receivedBuffers.isFull()) {
                freeBuffers.unshift(receivedBuffers.pop());
            }

            // Keep this packet so we can check if packets received later are duplicates
            receivedBuffers.unshift(p);
        } else {
            // Not a valid packet or is a duplicate packet, so return it immediately
            freeBuffers.unshift(p);
        }
    }

    interrupts();
}

//IRQ routine used to fill buffer of max size packets -> needs to be short and fast
void PacketReceiver::read() {
  bool tx_ds, tx_df, rx_dr;
  radio->whatHappened(tx_ds, tx_df, rx_dr); // resets the IRQ pin to HIGH
  uint8_t pipe = 0;
  
  while (radio->available(&pipe)) {
    if (!pendingBuffers.isFull() && !freeBuffers.isEmpty()) {
      uint8_t* p = freeBuffers.pop();
      radio->read(p, BUFFER_SIZE);
      pendingBuffers.unshift(p);
    } else {
      // Not enough buffers to store the packet
      radio->flush_rx();
      break;
    }
  }
}

void PacketReceiver::setBufferCallback(std::function<void(const uint8_t*)> callback) {
  this->bufferCallback = callback;
}

void PacketReceiver::setInvalidBufferCallback(std::function<void(const uint8_t*)> callback)
{
  this->invalidBufferCallback = callback;
}

bool PacketReceiver::isSaneBuffer(uint8_t *buffer) {
  // Sanity check that the buffer isn't just filled with noise
  // First byte contains a header
  // Second byte contains the packet length which cannot be larger than 28 bytes (32 bytes - 1 header byte - 1 length byte - 2 CRC bytes)
  if (buffer[0] == 0xC0 && buffer[1] <= 28) {
      return true;
  }
  return false;
}

bool PacketReceiver::isValidBuffer(uint8_t *buffer) {
  if (isSaneBuffer(buffer)) {
      if (packetCRC.check(buffer)) {
          return true;
      }
  }
  return false;
}

bool PacketReceiver::isNewBuffer(uint8_t *buffer) {
  for (int i = 0; i < receivedBuffers.size(); i++) {
    if (isEquivalentBuffer(buffer, receivedBuffers[i])) {
      return false;
    }
  }
  return true;
}

bool PacketReceiver::isEquivalentBuffer(uint8_t *a, uint8_t *b) {
  // Check length byte
  if (a[1] != b[1]) {
    return false;
  }
  // TODO: Could we just check the source and destination address and the rolling code?
  uint8_t length = a[1] + 4;
  for (int i = 0; i < length; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}
