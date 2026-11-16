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
  UNKNOWN
};

enum class FieldType {
  VALUE,
  SET,
  FETCH,
  UNKNOWN
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

using PacketParameters = std::variant<std::monostate, FieldsParameters, SceneParameters>;

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

  bool isSameCommand(const Packet& other) const {
    return rollingCode2 == other.rollingCode2 &&
           type == other.type &&
           header == other.header &&
           parameters == other.parameters;
  }
};


#endif // PACKET_TYPES_H