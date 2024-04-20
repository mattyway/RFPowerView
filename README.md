# RFPowerView

Library for sending and receiving PowerView packets so that blinds can be controlled programatically. PowerView is a brand of remote controlled blinds developed by Hunter Douglas (also known as Luxaflex in some regions). This library is intended to be used on a microcontroller like an ESP8266 in conjunction with a nRF24L01 transceiver module. The implementation has been entirely reverse engineered by sniffing packets sent from a PowerView Hub or Pebble remote.

## Supported commands
- Open
- Close
- Stop
- Open slowly
- Close slowly
- Move to saved position
- Move to position
- Query position
- Query battery level

## Code Sample

```
#include <Arduino.h>
#include <RFPowerView.h>

// These define which pins should be used to communicate with the nRF24L01 board
#define RF_CE_PIN   5
#define RF_CS_PIN   15
#define RF_IRQ_PIN  4

// Copied from Powerview Hub userdata API. Open http://POWERVIEW_HUB_IP_ADDRESS/api/userdata/ and find the field labeled "rfID"
#define RF_ID       0x2ECB

RFPowerView powerView(RF_CE_PIN, RF_CS_PIN, RF_IRQ_PIN, RF_ID);

void processPacket(const Packet*);

void setup() {
  powerView.setPacketReceivedCallback(processPacket);
  if (!powerView.begin()) {
    // Failed to start RFPowerView
    return;
  }
}

void loop() {
  powerView.loop();
}

void processPacket(const Packet *packet) {
    // TODO: React to packet
}

```

See [PowerViewSniffer](https://github.com/mattyway/PowerViewSniffer/) for a more complete example.

## Protocol

A PowerView packet consists of a header (which contains a variable length address section), a variable length payload, and a two byte checksum at the end. Each packet is limited to being at most 32 bytes due to limitations of the nRF24L01 module.

### Header

| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | ID Byte | Identifies the start of a new packet. | `0xCO` |
| 1 | 8 | Length | Specifies how many bytes of header and data come after this byte (excludes checksum bytes). | `0x0A` - `0x1D` |
| 2 | 8 | Unknown | Could indicate the type of sender | `0x00` (from hub or remote), `0x01` (from blind) |
| 3 | 8 | Fixed | This byte is always the same. | `0x05` |
| 4 | 8 | Rolling Code 1 | This byte is incremented by 0x01 each time a packet is sent. | `0x00`-`0xFF` |
| 5 | 16 | Fixed | These two bytes are always the same. | `0xFFFF` |
| 7 | 16 | Physical Source ID | The ID of device that actually sent the packet (can be different to Logical Source ID when a repeater is being used) | `0x0000`-`0xFFFF` |
| 9 | 8 | Unknown | This byte might indicate if the packet was retransmitted by a repeater | `0x86` (when sent from hub or remote), `0x85` (when sent from repeater) |
| 10 | 8 | Address Type | Indicates which type of address this packet is using. | `0x04` (Broadcast), `0x05` (Unicast), `0x06` (Groups) |
| 11 | 8 | Rolling Code 2 | This byte is incremented by 0x01 each time a packet is sent. | `0x00`-`0xFF` |
| 12 | Varies | Address | This section is a variable length. | Broadcast, Unicast, or Groups section (see next section) |

#### Broadcast address
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 16 | Logical Source ID | The ID of device where the packet originated (hubs seem to always be 0x0000) | `0x0000`-`0xFFFF` |

Note: this type of address seems to be used when a hub is activating a scene (and multiple blinds could be included in the scene).

#### Unicast address
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 16 | Destination ID | The ID of device that should process the packet | `0x0000`-`0xFFFF` |
| 2 | 16 | Logical Source ID | The ID of device where the packet originated (hubs seem to always be 0x0000) | `0x0000`-`0xFFFF` |

Note: this type of address is used when a hub wants to communicate with a specific blind or when a blind is communicating with a hub.

#### Groups address
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0-5 | 8 | Group Number | Devices assigned to the specified group numbers should process the packet. | `0x01`-`0x06` |
| n | 8 | Terminator | Marker to stop processing group numbers | `0x00` |
| n+1 | 16 | Logical Source ID | The ID of device where the packet originated | `0x0000`-`0xFFFF` |

Note: this type of address is used by pebble remotes. The Group Number corresponds to the the numbered buttons on the pebble remote. When multiple group numbers are included, generally they are in ascending order.

### Payload
This section begins at the next byte following the variable length address section. I've observed two types of payloads: simple and "fields".

### Simple payload
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Packet Type | Indicates the payload is a simple command | `0x52` |
| 1 | 8 | Command | What should the blind do | `0x55` (Open), `0x44` (Close), `0x53` (Stop), `0x52` (Open Slowly), `0x4C` (Close Slowly), `0x48` (Move to saved position) |
| 2 | 8 | End | Marker to indicate end of data | `0x00` |

Note: this type of payload is used by both the hub and pebble remotes. Used for commands like "Open", "Close", or "Stop"

#### Fields payload
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Field Mode | Indicates the payloads contains "fields" | `0x3F` (fields without values), `0x21` (fields with values) |
| 1 | 8 | Fixed | This byte is always the same. | `0x5A` |
| 2 | varies | Field data | This section is a variable length depending on the number of fields the payload contains | Field data (see next section) |

Note: this type of payload is used by the hub. If sent to a blind without values, it will prompt the blind to respond with the value of those fields. If sent to a blind with values, it will prompt the blind to "apply" the value to the fields (eg. by moving to a position). A single payload can contain multiple fields.

#### Field data
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Field Length | Indicates how many bytes are in this field | `0x02`-`0x04` |
| 1 | 8 | Field Mode | Seems to always be the same as the first Field Mode | `0x3F` (fields without values), `0x21` (fields with values) |
| 2 | 8 | Field ID | The field ID | `0x00`-`0xFF`, eg. `0x50` is Position, `0x42` is Battery level |
| 3 (if length=3) | 8 | Field Value | The value of the field | `0x00`-`0xFF` |
| 3 (if length=4) | 16 | Field Value | The value of the field | `0x0000`-`0xFFFF` |

### Checksum
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 16 | Checksum | A CRC16 checksum of the packet data | `0x0000`-`0xFFFF` |

## Sample Packets

### Open blinds in group 4

```
C01100056CFFFF369E86063C0400369E525500B988
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x11` |  |
| 4 | Rolling Code 1 | `0x6C` |  |
| 7 | Physical Source ID | `0x369E` | ID of pebble remote |
| 10 | Address type | `0x06` | Using Groups address type |
| 11 | Rolling Code 2 | `0x3C` |  |
| 12 | Groups | 4 | Only a single group specified |
| 14 | Logical Source ID | `0x369E` | ID of pebble remote (same as Physical Source ID) |
| 19 | Checksum | `0xB988` | |


### Hub requesting values from a blind

```
C019000592FFFF72CB85054E4EF100003F5A023F50023F4D023F54C9F3
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x19` |  |
| 4 | Rolling Code 1 | `0x92` |  |
| 7 | Physical Source ID | `0x72CB` | ID of repeater |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0x4E` |  |
| 12 | Destination ID | `0x4EF1` | ID of blind |
| 14 | Logical Source ID | `0x0000` | ID of hub (different than Physical Source ID) |
| 18 | Field 1 | `023F50` | Field of 2 bytes, ID = `0x50` (position) |
| 21 | Field 2 | `023F4D` | Field of 2 bytes, ID = `0x4D` (unknown) |
| 24 | Field 3 | `023F54` | Field of 2 bytes, ID = `0x54` (unknown) |
| 27 | Checksum  | `0xC9F3` |  |

### Blind reporting position value to hub

```
C0151005E0FFFF4EF186051A00004EF1215A04215040016670
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x15` |  |
| 4 | Rolling Code 1 | `0xE0` |  |
| 7 | Physical Source ID | `0x4EF1` | ID of blind |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0x1A` |  |
| 12 | Destination ID | `0x0000` | ID of hub |
| 14 | Logical Source ID | `0x4EF1` | ID of blind |
| 18 | Field 1 | `0421504001` | Field of 4 bytes, ID = `0x50` (position), value = `0x4001` |
| 23 | Checksum  | `0x6670` |  |


### Blind reporting battery level value to hub

```
C014100558FFFF4EF18605C100004EF1215A0321429DEC23
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x14` |  |
| 4 | Rolling Code 1 | `0x58` |  |
| 7 | Physical Source ID | `0x4EF1` | ID of blind |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0xC1` |  |
| 12 | Destination ID | `0x0000` | ID of hub |
| 14 | Logical Source ID | `0x4EF1` | ID of blind |
| 18 | Field 1 | `0321429D` | Field of 3 bytes, ID = `0x42` (battery level), value = `0x9D` |
| 22 | Checksum  | `0xEC23` |  |

### Hub activating a scene

```
C00F0005A1FFFF00008604FF000053471B446B
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x0F` |  |
| 4 | Rolling Code 1 | `0xA1` |  |
| 7 | Physical Source ID | `0x0000` | ID of hub |
| 10 | Address type | `0x04` | Using Broadcast address type |
| 11 | Rolling Code 2 | `0xFF` |  |
| 12 | Logical Source ID | `0x0000` | ID of hub |
| 16 | Scene ID | `1B` |  |
| 17 | Checksum  | `0x446B` |  |

Note: activating scenes is not supported by the library (yet!)
