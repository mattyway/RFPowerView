#ifndef PACKET_TYPES_H
#define PACKET_TYPES_H

#include <stdint.h>
#include <variant>

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
};

struct FieldsParameters {
  std::vector<Field> fields;
};

struct SceneParameters {
  uint8_t sceneID;
};

using ActivateSceneParameters = SceneParameters;
using StoreSceneParameters = SceneParameters;

using PacketParameters = std::variant<std::monostate, FieldsParameters, SceneParameters>;

struct BroadcastHeader {
  uint16_t source;
};

struct UnicastHeader {
  uint16_t source;
  uint16_t destination;
};

struct GroupsHeader {
  uint16_t source;
  std::vector<uint8_t> groups;
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
};


#endif // PACKET_TYPES_H