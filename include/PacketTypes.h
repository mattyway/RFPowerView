#ifndef PACKET_TYPES_H
#define PACKET_TYPES_H

#include <stdint.h>
#include <variant>
#include <vector>

// Define packet types
enum class PacketType {
  OPEN,
  CLOSE,
  STOP,
  OPEN_SLOW,
  CLOSE_SLOW,
  MOVE_TO_SAVED_POSITION,
  FIELDS,
  FIELD_COMMAND,
  ACTIVATE_SCENE,
  STORE_SCENE,
  GET_FIRMWARE,
  REPORT_FIRMWARE,
  UNKNOWN
};

enum class FirmwareTarget : uint8_t {
  BLIND_CONTROLLER = 0x4E, // ASCII 'N' (Main controller firmware)
  MOTOR_CONTROLLER = 0x43  // ASCII 'C' (Motor controller firmware)
};

struct FirmwareParameters {
  FirmwareTarget target{FirmwareTarget::BLIND_CONTROLLER};
  uint8_t major{0};
  uint8_t minor{0};
  uint16_t build{0};

  bool operator==(const FirmwareParameters& o) const {
    return target == o.target && major == o.major && minor == o.minor && build == o.build;
  }
  bool operator!=(const FirmwareParameters& o) const { return !(*this == o); }

  void getAscii(char dest[5]) const {
    dest[0] = static_cast<char>(build & 0xFF);
    dest[1] = static_cast<char>((build >> 8) & 0xFF);
    dest[2] = static_cast<char>(minor);
    dest[3] = static_cast<char>(major);
    dest[4] = '\0';
  }

  void setAscii(const char src[4]) {
    build = static_cast<uint16_t>(static_cast<uint8_t>(src[0]) | (static_cast<uint8_t>(src[1]) << 8));
    minor = static_cast<uint8_t>(src[2]);
    major = static_cast<uint8_t>(src[3]);
  }
};

enum class FieldType {
  VALUE,
  SET,
  FETCH,
  UNKNOWN
};

enum class SenderType : uint8_t {
  STANDARD = 0x00,
  BLIND_AT_REST = 0x10,
};

struct Field {
  uint8_t identifier;
  FieldType type;
  bool hasValue;
  std::variant<std::monostate, uint8_t, uint16_t> value;

  bool operator==(const Field& o) const {
    return identifier == o.identifier &&
           type == o.type &&
           hasValue == o.hasValue &&
           value == o.value;
  }
  bool operator!=(const Field& o) const { return !(*this == o); }
};

struct FieldsParameters {
  std::vector<Field> fields;

  bool operator==(const FieldsParameters& o) const { return fields == o.fields; }
  bool operator!=(const FieldsParameters& o) const { return !(*this == o); }
};

struct SceneParameters {
  uint8_t sceneID;

  bool operator==(const SceneParameters& o) const { return sceneID == o.sceneID; }
  bool operator!=(const SceneParameters& o) const { return !(*this == o); }
};

using ActivateSceneParameters = SceneParameters;
using StoreSceneParameters = SceneParameters;

using PacketParameters = std::variant<std::monostate, FieldsParameters, SceneParameters, FirmwareParameters>;

struct BroadcastHeader {
  uint16_t source;

  bool operator==(const BroadcastHeader& o) const { return source == o.source; }
  bool operator!=(const BroadcastHeader& o) const { return !(*this == o); }
};

struct UnicastHeader {
  uint16_t source;
  uint16_t destination;

  bool operator==(const UnicastHeader& o) const {
    return source == o.source && destination == o.destination;
  }
  bool operator!=(const UnicastHeader& o) const { return !(*this == o); }
};

struct GroupsHeader {
  uint16_t source;
  std::vector<uint8_t> groups;

  bool operator==(const GroupsHeader& o) const {
    return source == o.source && groups == o.groups;
  }
  bool operator!=(const GroupsHeader& o) const { return !(*this == o); }
};

using PacketHeader = std::variant<BroadcastHeader, UnicastHeader, GroupsHeader>;

// Define Packet structure
struct Packet {
  PacketHeader header;  
  PacketType type;
  PacketParameters parameters;
  uint8_t rollingCode1;
  uint8_t rollingCode2;
  uint16_t physicalSource = 0;
  bool repeated = false;
  SenderType senderType{SenderType::STANDARD};

  bool isSameCommand(const Packet& other) const {
    return rollingCode2 == other.rollingCode2 &&
           type == other.type &&
           header == other.header &&
           parameters == other.parameters;
  }
};


#endif // PACKET_TYPES_H