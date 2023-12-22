#ifndef PACKET_TYPES_H
#define PACKET_TYPES_H

#include <Arduino.h>
#include <variant>

// Define packet types
enum class PacketType {
  OPEN,
  CLOSE,
  STOP,
  FIELDS,
  FIELD_COMMAND,
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

using PacketParameters = std::variant<std::monostate, FieldsParameters>;


// Define Message structure
struct Packet {
  uint16_t source;
  uint16_t destination;
  PacketType type;
  PacketParameters parameters;
};


#endif // PACKET_TYPES_H