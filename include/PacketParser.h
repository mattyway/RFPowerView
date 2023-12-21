#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

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

class PacketParser {
public:
    PacketParser();
    ~PacketParser();

    bool parsePacket(const uint8_t *buffer, Packet& message);
private:
    bool parseFields(const uint8_t *buffer, std::vector<Field>& fields);
};

#endif // PACKET_PARSER_H