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
- Query shade type
- Query blind and motor firmware versions
- Activate scene
- Store scene

## Code Sample

```
#include <Arduino.h>
#include <RFPowerView.h>

// These define which pins should be used to communicate with the nRF24L01 board
#define RF_CE_PIN   5
#define RF_CS_PIN   15
#define RF_IRQ_PIN  4

// Copied from PowerView Hub userdata API. Open http://POWERVIEW_HUB_IP_ADDRESS/api/userdata/ and find the field labeled "rfID"
#define RF_ID       0x2EC8

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
| 0 | 8 | ID Byte | Identifies the start of a new packet. | `0xC0` |
| 1 | 8 | Length | Specifies how many bytes of header and data come after this byte (excludes checksum bytes). | `0x0A` - `0x1D` |
| 2 | 8 | Sender Type | Indicates sender device role | `0x00` (Controller: hub, remote, repeater), `0x10` (Shade motor) |
| 3 | 8 | Fixed | This byte is always the same. | `0x05` |
| 4 | 8 | Rolling Code 1 | Set by the transmitting physical device (`physicalSource`). Originating devices (such as the hub) typically increment this for each new command, while repeaters replace it with their own transmission counter. All copies in a burst from the same physical device share this value. | `0x00`-`0xFF` |
| 5 | 16 | Fixed | These two bytes are always the same. | `0xFFFF` |
| 7 | 16 | Physical Source ID | The ID of the device that actually transmitted this copy (a repeater when relayed) | `0x0000`-`0xFFFF` |
| 9 | 8 | Repeat Flag | Indicates if the packet was retransmitted by a repeater | `0x86` (direct), `0x85` (relayed by repeater) |
| 10 | 8 | Address Type | Indicates which type of address this packet is using. | `0x04` (Broadcast), `0x05` (Unicast), `0x06` (Groups) |
| 11 | 8 | Rolling Code 2 | Set by the originating logical sender (`logicalSource`) as a per-command identifier. Originating devices (such as the hub) typically increment this for each new command. Repeaters preserve this value unchanged. | `0x00`-`0xFF` |
| 12 | Varies | Address | This section is a variable length. | Broadcast, Unicast, or Groups section (see next section) |

Note on rolling codes and relayed copies:
- Originating devices generate a pair of rolling codes for each command or transaction. While the hub increments both sequentially by `0x01`, pebble remotes have been observed generating values that appear pseudorandom rather than sequential counters.
- **Rolling Code 1** tracks the physical transmitter: repeaters substitute their own counter while preserving payload and address.
- **Rolling Code 2** tracks the logical command: repeaters (and relaying secondary hubs) leave it unchanged.
- When a secondary PowerView hub relays commands, its copy keeps Physical Source ID `0x0000` and direct flag `0x86` in byte 9, while using its own Rolling Code 1 and preserving Rolling Code 2.
- Copies of one command (direct, repeater, or second hub) will share the same destination (or group numbers), Logical Source ID, payload, and Rolling Code 2. The library provides `Packet::isSameCommand(const Packet& other)` to match these duplicate copies across transmissions.

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
| 0-5 | 8 | Group Number | Devices assigned to the specified group numbers should process the packet. | `0x01`-`0x06` (could be higher values when sent from hub) |
| n | 8 | Terminator | Marker to stop processing group numbers | `0x00` |
| n+1 | 16 | Logical Source ID | The ID of device where the packet originated | `0x0000`-`0xFFFF` |

Note: this type of address is used by pebble remotes. The Group Number corresponds to the numbered buttons on the pebble remote. When multiple group numbers are included, generally they are in ascending order. When sent from a hub (with Logical Source ID `0x0000`), group numbers can be higher values (such as `0x20` for room-wide commands).

#### Device IDs and Roles

All 16-bit device IDs (`Physical Source ID`, `Logical Source ID`, and `Destination ID`) are transmitted big-endian.

- **Hub (`0x0000`)**: The primary hub always uses ID `0x0000`. It communicates with shades via Unicast or scene Broadcasts, and can issue room-wide commands via Groups.
- **Blinds / Shades**: A blind's radio ID is the shade `id` from the hub's HTTP API (`/api/shades`) with its two bytes swapped (eg. an API `id` of `61774` (`0xF14E`) would be radio ID `0x4EF1`). Blinds accept unicast commands from the hub, but ignore unicast commands originating from remote IDs.
- **Pebble Remotes**: Remotes have unique 16-bit IDs and control shades using Groups addressing (buttons 1–6) rather than Unicast.
- **Repeaters**: Repeaters have their own 16-bit IDs. When retransmitting a packet, a repeater inserts its own ID into `Physical Source ID` while preserving the originating device's `Logical Source ID`.

### Payload
This section begins at the next byte following the variable length address section. Four types of payloads are supported: simple, properties, scene, and firmware.

#### Simple payload
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Packet Type | Indicates the payload is a simple command | `0x52` |
| 1 | 8 | Command | What should the blind do | `0x55` (Open), `0x44` (Close), `0x53` (Stop), `0x52` (Open Slowly), `0x4C` (Close Slowly), `0x48` (Move to saved position) |
| 2 | 8 | End | Marker to indicate end of data | `0x00` |

Note: this type of payload is used by both the hub and pebble remotes. Used for commands like "Open", "Close", or "Stop".

#### Properties payload
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Packet Type | Indicates the payload contains properties | `0x3F` (GET/SET command), `0x21` (REPORT) |
| 1 | 8 | Fixed | This byte is always the same. | `0x5A` |
| 2 | varies | Property data | This section is a variable length depending on the number of properties the payload contains | Property data (see next section) |

Note: this type of payload is used by the hub and shades. If sent to a shade without values (`GET_PROPERTIES`), it prompts the shade to respond with the current values of those properties. If sent to a shade with values (`SET_PROPERTIES`), it prompts the shade to apply those values (eg. by moving to a position). A single payload can contain multiple properties.

##### Property data
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Property Length | Indicates how many bytes are in this property entry | `0x02`-`0x04` |
| 1 | 8 | Property Type | Indicates how value should be used | `0x3F` (fetch/get), `0x40` (set), `0x21` (value/report) |
| 2 | 8 | Property ID | The property identifier | `0x00`-`0xFF`, eg. `0x50` is Position, `0x42` is Battery level, `0x4D` is Tilt, `0x54` is Shade type |
| 3 (if length=3) | 8 | Property Value | The value of the property | `0x00`-`0xFF` |
| 3 (if length=4) | 16 | Property Value | The value of the property, little-endian (low byte first) | `0x0000`-`0xFFFF` |

Note: 16-bit property values are little-endian, eg. property `0421504001` has the value `0x0140`.

#### Scene payload
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Packet Type | Indicates the payload is a scene command | `0x53` |
| 1 | 8 | Command | What scene action to perform | `0x47` (Activate Scene), `0x43` (Store Scene) |
| 2 | 8 | Scene ID | The ID of the scene | `0x00`-`0xFF` |

Note: `0x47` (Activate Scene) is typically broadcast by the hub to trigger all blinds assigned to a scene. `0x43` (Store Scene) is sent unicast by the hub to individual blinds when configuring a scene.

#### Firmware payload
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 8 | Packet Type | Indicates the payload is a firmware query or report | `0x3F` (GET_FIRMWARE), `0x21` (REPORT_FIRMWARE) |
| 1 | 8 | Fixed | Subcommand identifier for firmware | `0x46` |
| 2 | 8 | Target | Controller target | `0x4E` (Blind Controller), `0x43` (Motor Controller) |
| 3-6 (if report) | 32 | Firmware Version | 4-byte version info | Little-endian build `uint16`, minor `uint8`, major `uint8` |

Note: in firmware query packets (`0x3F 0x46`), byte 2 indicates the target (`0x4E` or `0x43`). In report packets (`0x21 0x46`), bytes 3–6 contain the 4-byte version encoding where `build = (b1 << 8) | b0`, `minor = b2`, and `major = b3`.

### Checksum
| Byte Offset | Size (Bits) | Name | Description | Valid Values |
|---|---|---|---|---|
| 0 | 16 | Checksum | A CRC16 checksum of the packet data, high byte first | `0x0000`-`0xFFFF` |

The checksum is CRC-16 with polynomial `0x755B`, initial value `0xFFFF`, no reflection and no final XOR. It is calculated over the network's 2-byte radio address (the hub's `rfID`, high byte first) followed by packet bytes 0 to L+1 (everything except the checksum). The `rfID` itself is not transmitted.

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
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x06` | Using Groups address type |
| 11 | Rolling Code 2 | `0x3C` |  |
| 12 | Groups | 4 | Only a single group specified |
| 13 | Terminator | `0x00` | Marker to stop processing group numbers |
| 14 | Logical Source ID | `0x369E` | ID of pebble remote (same as Physical Source ID) |
| 16 | Packet Type | `0x52` | Simple command |
| 17 | Command | `0x55` | Open |
| 18 | End | `0x00` | Marker to indicate end of data |
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
| 9 | Repeat Flag | `0x85` | Packet retransmitted by repeater |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0x4E` |  |
| 12 | Destination ID | `0x4EF1` | ID of blind |
| 14 | Logical Source ID | `0x0000` | ID of hub (different than Physical Source ID) |
| 18 | Property 1 | `023F50` | Property of 2 bytes, Type = `0x3F` (fetch), ID = `0x50` (position) |
| 21 | Property 2 | `023F4D` | Property of 2 bytes, Type = `0x3F` (fetch), ID = `0x4D` (tilt position) |
| 24 | Property 3 | `023F54` | Property of 2 bytes, Type = `0x3F` (fetch), ID = `0x54` (shade type) |
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
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0x1A` |  |
| 12 | Destination ID | `0x0000` | ID of hub |
| 14 | Logical Source ID | `0x4EF1` | ID of blind |
| 18 | Property 1 | `0421504001` | Property of 4 bytes, Type = `0x21` (value), ID = `0x50` (position), Value = `0x0140` (little-endian) |
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
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0xC1` |  |
| 12 | Destination ID | `0x0000` | ID of hub |
| 14 | Logical Source ID | `0x4EF1` | ID of blind |
| 18 | Property 1 | `0321429D` | Property of 3 bytes, Type = `0x21` (value), ID = `0x42` (battery level), Value = `0x9D` |
| 22 | Checksum  | `0xEC23` |  |

### Hub setting position of a blind

```
C015000529FFFF00008605B94EF100003F5A044050FFFF286B
```
| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x15` |  |
| 4 | Rolling Code 1 | `0x29` |  |
| 7 | Physical Source ID | `0x0000` | ID of hub |
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0xB9` |  |
| 12 | Destination ID | `0x4EF1` | ID of blind |
| 14 | Logical Source ID | `0x0000` | ID of hub |
| 18 | Property 1 | `044050FFFF` | Property of 4 bytes, Type = `0x40` (set), ID = `0x50` (position), Value = `0xFFFF` (100% open) |
| 23 | Checksum  | `0x286B` |  |

### Hub activating a scene

```
C00F0005A1FFFF00008604FF000053471B446B
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x0F` |  |
| 4 | Rolling Code 1 | `0xA1` |  |
| 7 | Physical Source ID | `0x0000` | ID of hub |
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x04` | Using Broadcast address type |
| 11 | Rolling Code 2 | `0xFF` |  |
| 12 | Logical Source ID | `0x0000` | ID of hub |
| 14 | Packet Type | `0x53` | Scene command |
| 15 | Command | `0x47` | Activate Scene |
| 16 | Scene ID | `0x1B` |  |
| 17 | Checksum  | `0x446B` |  |

### Hub requesting firmware from a blind

```
C0110005C1FFFF000086056A4EF100003F464EBA02
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x11` |  |
| 4 | Rolling Code 1 | `0xC1` |  |
| 7 | Physical Source ID | `0x0000` | ID of hub |
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0x6A` |  |
| 12 | Destination ID | `0x4EF1` | ID of blind |
| 14 | Logical Source ID | `0x0000` | ID of hub |
| 16 | Packet Type | `0x3F` | Query command |
| 17 | Subcommand | `0x46` | Firmware query |
| 18 | Target | `0x4E` | Blind Controller |
| 19 | Checksum | `0xBA02` |  |

### Blind reporting controller firmware version to hub

```
C015100594FFFF4EF18605AB00004EF121464E980708018DE7
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x15` |  |
| 4 | Rolling Code 1 | `0x94` |  |
| 7 | Physical Source ID | `0x4EF1` | ID of blind |
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0xAB` |  |
| 12 | Destination ID | `0x0000` | ID of hub |
| 14 | Logical Source ID | `0x4EF1` | ID of blind |
| 16 | Packet Type | `0x21` | Report |
| 17 | Subcommand | `0x46` | Firmware report |
| 18 | Target | `0x4E` | Blind Controller |
| 19 | Build | `0x0798` | Value = 1944 (little-endian: `0x98`, `0x07`) |
| 21 | Minor | `0x08` | Value = 8 |
| 22 | Major | `0x01` | Value = 1 (Version 1.8.1944) |
| 23 | Checksum | `0x8DE7` |  |

### Blind reporting motor firmware version to hub

```
C0151005A0FFFF4EF18605B700004EF1214643322E3030EF05
```

| Byte Offset | Name | Value | Notes |
|---|---|---|---|
| 1 | Length | `0x15` |  |
| 4 | Rolling Code 1 | `0xA0` |  |
| 7 | Physical Source ID | `0x4EF1` | ID of blind |
| 9 | Repeat Flag | `0x86` | Direct transmission |
| 10 | Address type | `0x05` | Using Unicast address type |
| 11 | Rolling Code 2 | `0xB7` |  |
| 12 | Destination ID | `0x0000` | ID of hub |
| 14 | Logical Source ID | `0x4EF1` | ID of blind |
| 16 | Packet Type | `0x21` | Report |
| 17 | Subcommand | `0x46` | Firmware report |
| 18 | Target | `0x43` | Motor Controller |
| 19 | Build | `0x2E32` | Value = 11826 (little-endian: `0x32`, `0x2E`) |
| 21 | Minor | `0x30` | Value = 48 |
| 22 | Major | `0x30` | Value = 48 (Version 48.48.11826) |
| 23 | Checksum | `0xEF05` |  |

Note: The official PowerView app displays this version as `48.48.11826` following the standard `major.minor.build` representation, but the four raw bytes (`0x32`, `0x2E`, `0x30`, `0x30`) could also be interpreted as the ASCII string `"2.00"`.
