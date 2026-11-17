#include <unity.h>
#include <Arduino.h>
#include "../hex_helper.h"
#include "PacketParser.h"

void setUp()
{
}

void tearDown()
{
}

void run_parse_test(const uint8_t *packet_data, Packet &packet)
{
  PacketParser packetParser;

  bool result = packetParser.parsePacket(packet_data, packet);

  TEST_ASSERT_TRUE_MESSAGE(result, "Unable to parse packet_data");
}

void test_parse_stop()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::STOP);

  delete[] packet_data;
}

void test_parse_close()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E524400");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::CLOSE);

  delete[] packet_data;
}

void test_parse_open()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525500");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::OPEN);

  delete[] packet_data;
}

void test_parse_rolling_codes()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");
  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x6C, packet.rollingCode1, "Rolling Code 1");
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x3C, packet.rollingCode2, "Rolling Code 2");

  delete[] packet_data;
}

void test_groups_source_address()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<GroupsHeader>(packet.header);

  TEST_ASSERT_EQUAL_HEX16(0x369E, header.source);

  delete[] packet_data;
}

void test_groups_single_group()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<GroupsHeader>(packet.header);

  TEST_ASSERT_EQUAL_MESSAGE(1, header.groups.size(), "Group count");
  TEST_ASSERT_EQUAL_INT8_MESSAGE(4, header.groups[0], "Group at offset 0");

  delete[] packet_data;
}

void test_groups_multiple_groups()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C03040500369E525300");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<GroupsHeader>(packet.header);

  TEST_ASSERT_EQUAL_MESSAGE(3, header.groups.size(), "Group count");
  TEST_ASSERT_EQUAL_INT8_MESSAGE(3, header.groups[0], "Group at offset 0");
  TEST_ASSERT_EQUAL_INT8_MESSAGE(4, header.groups[1], "Group at offset 1");
  TEST_ASSERT_EQUAL_INT8_MESSAGE(5, header.groups[2], "Group at offset 2");

  delete[] packet_data;
}

void test_unicast_source_address()
{
  const uint8_t* packet_data = hex_string_to_array("C019000592FFFF72CB85054E4EF100003F5A023F50023F4D023F54");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<UnicastHeader>(packet.header);

  TEST_ASSERT_EQUAL_HEX16(0x0000, header.source);

  delete[] packet_data;
}

void test_unicast_destination_address()
{
  const uint8_t* packet_data = hex_string_to_array("C019000592FFFF72CB85054E4EF100003F5A023F50023F4D023F54");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<UnicastHeader>(packet.header);

  TEST_ASSERT_EQUAL_HEX16(0x4EF1, header.destination);

  delete[] packet_data;
}

void test_broadcast_source_address()
{
  const uint8_t* packet_data = hex_string_to_array("C00F0005A1FFFF00008604FF000053471B446B");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<BroadcastHeader>(packet.header);

  TEST_ASSERT_EQUAL_HEX16(0x0000, header.source);

  delete[] packet_data;
}

void test_parse_activate_scene()
{
  const uint8_t* packet_data = hex_string_to_array("C00F0005A1FFFF00008604FF000053471B446B");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto parameters = std::get<ActivateSceneParameters>(packet.parameters);

  TEST_ASSERT_EQUAL_HEX8(0x1B, parameters.sceneID);

  delete[] packet_data;
}

void test_parse_store_scene()
{
  const uint8_t* packet_data = hex_string_to_array("C011000529FFFF00008605B94EF1000053431B");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::STORE_SCENE);

  auto parameters = std::get<StoreSceneParameters>(packet.parameters);

  TEST_ASSERT_EQUAL_HEX8(0x1B, parameters.sceneID);

  delete[] packet_data;
}

void test_parse_fields()
{
  const uint8_t* packet_data = hex_string_to_array("C019000592FFFF72CB85054E4EF100003F5A023F50023F4D023F54C9F3");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto parameters = std::get<FieldsParameters>(packet.parameters);

  TEST_ASSERT_EQUAL_INT(3, parameters.fields.size());

  TEST_ASSERT_EQUAL_HEX8(0x50, parameters.fields[0].identifier);
  TEST_ASSERT_FALSE_MESSAGE(parameters.fields[0].hasValue, "Field 0 should not have a value.");

  TEST_ASSERT_EQUAL_HEX8(0x4D, parameters.fields[1].identifier);
  TEST_ASSERT_FALSE_MESSAGE(parameters.fields[1].hasValue, "Field 1 should not have a value.");

  TEST_ASSERT_EQUAL_HEX8(0x54, parameters.fields[2].identifier);
  TEST_ASSERT_FALSE_MESSAGE(parameters.fields[2].hasValue, "Field 2 should not have a value.");

  delete[] packet_data;
}

void test_parse_fields_data_uint16()
{
  const uint8_t* packet_data = hex_string_to_array("C0151005E0FFFF4EF186051A00004EF1215A04215040016670");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto parameters = std::get<FieldsParameters>(packet.parameters);

  TEST_ASSERT_EQUAL_INT(1, parameters.fields.size());

  TEST_ASSERT_EQUAL_HEX8(0x50, parameters.fields[0].identifier);
  TEST_ASSERT_TRUE_MESSAGE(parameters.fields[0].hasValue, "Field 0 should have a value.");
  TEST_ASSERT_EQUAL_HEX16(0x0140, std::get<uint16_t>(parameters.fields[0].value));

  delete[] packet_data;
}

void test_parse_fields_data_uint8()
{
  const uint8_t* packet_data = hex_string_to_array("C014100558FFFF4EF18605C100004EF1215A0321429DEC23");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto parameters = std::get<FieldsParameters>(packet.parameters);

  TEST_ASSERT_EQUAL_INT(1, parameters.fields.size());

  TEST_ASSERT_EQUAL_HEX8(0x42, parameters.fields[0].identifier);
  TEST_ASSERT_TRUE_MESSAGE(parameters.fields[0].hasValue, "Field 0 should have a value.");
  TEST_ASSERT_EQUAL_HEX8(0x9D, std::get<uint8_t>(parameters.fields[0].value));

  delete[] packet_data;
}

void test_parse_fields_multiple_groups()
{
  const uint8_t* packet_data = hex_string_to_array("C01600056CFFFF369E86063C040500369E3F5A04405040012FCC");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::FIELD_COMMAND);

  auto parameters = std::get<FieldsParameters>(packet.parameters);

  TEST_ASSERT_EQUAL_INT(1, parameters.fields.size());

  TEST_ASSERT_EQUAL_HEX8(0x50, parameters.fields[0].identifier);
  TEST_ASSERT_TRUE_MESSAGE(parameters.fields[0].type == FieldType::SET, "Field 0 should be a set field.");
  TEST_ASSERT_TRUE_MESSAGE(parameters.fields[0].hasValue, "Field 0 should have a value.");
  TEST_ASSERT_EQUAL_HEX16(0x0140, std::get<uint16_t>(parameters.fields[0].value));

  delete[] packet_data;
}

void test_physical_source_from_repeater()
{
  const uint8_t* packet_data = hex_string_to_array("C019000592FFFF72CB85054E4EF100003F5A023F50023F4D023F54C9F3");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_EQUAL_HEX16(0x72CB, packet.physicalSource);
  TEST_ASSERT_TRUE_MESSAGE(packet.repeated, "Packet should be marked as repeated.");

  delete[] packet_data;
}

void test_physical_source_from_hub()
{
  const uint8_t* packet_data = hex_string_to_array("C00F0005A1FFFF00008604FF000053471B446B");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_EQUAL_HEX16(0x0000, packet.physicalSource);
  TEST_ASSERT_FALSE_MESSAGE(packet.repeated, "Packet should not be marked as repeated.");

  delete[] packet_data;
}

void test_physical_source_from_remote()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_EQUAL_HEX16(0x369E, packet.physicalSource);
  TEST_ASSERT_FALSE_MESSAGE(packet.repeated, "Packet should not be marked as repeated.");

  delete[] packet_data;
}

void test_is_same_command_matching_direct_and_repeated()
{
  // Direct packet from remote
  const uint8_t* direct_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");
  // Relayed packet from repeater: different physicalSource (0x72CB), repeated (0x85), different rollingCode1 (0x92)
  const uint8_t* repeated_data = hex_string_to_array("C011000592FFFF72CB85063C0400369E525300");

  Packet direct_packet;
  Packet repeated_packet;

  run_parse_test(direct_data, direct_packet);
  run_parse_test(repeated_data, repeated_packet);

  TEST_ASSERT_TRUE(direct_packet.isSameCommand(repeated_packet));
  TEST_ASSERT_TRUE(repeated_packet.isSameCommand(direct_packet));

  delete[] direct_data;
  delete[] repeated_data;
}

void test_is_same_command_different_rolling_code()
{
  const uint8_t* packet1_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");
  const uint8_t* packet2_data = hex_string_to_array("C01100056CFFFF369E86063D0400369E525300");

  Packet packet1;
  Packet packet2;

  run_parse_test(packet1_data, packet1);
  run_parse_test(packet2_data, packet2);

  TEST_ASSERT_FALSE(packet1.isSameCommand(packet2));

  delete[] packet1_data;
  delete[] packet2_data;
}

void test_is_same_command_different_command()
{
  // STOP command
  const uint8_t* stop_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");
  // CLOSE command
  const uint8_t* close_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E524400");

  Packet stop_packet;
  Packet close_packet;

  run_parse_test(stop_data, stop_packet);
  run_parse_test(close_data, close_packet);

  TEST_ASSERT_FALSE(stop_packet.isSameCommand(close_packet));

  delete[] stop_data;
  delete[] close_data;
}

void test_is_same_command_different_destination()
{
  // Group 4
  const uint8_t* group4_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");
  // Group 5
  const uint8_t* group5_data = hex_string_to_array("C01100056CFFFF369E86063C0500369E525300");

  Packet group4_packet;
  Packet group5_packet;

  run_parse_test(group4_data, group4_packet);
  run_parse_test(group5_data, group5_packet);

  TEST_ASSERT_FALSE(group4_packet.isSameCommand(group5_packet));

  delete[] group4_data;
  delete[] group5_data;
}

void test_parse_sender_type()
{
  const uint8_t* remote_packet = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");
  const uint8_t* blind_packet = hex_string_to_array("C0151005E0FFFF4EF186051A00004EF1215A04215040016670");

  Packet packet;
  run_parse_test(remote_packet, packet);
  TEST_ASSERT_TRUE(packet.senderType == SenderType::STANDARD);

  run_parse_test(blind_packet, packet);
  TEST_ASSERT_TRUE(packet.senderType == SenderType::BLIND_AT_REST);

  delete[] remote_packet;
  delete[] blind_packet;
}

int runUnityTests(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_parse_stop);
  RUN_TEST(test_parse_close);
  RUN_TEST(test_parse_open);
  RUN_TEST(test_parse_rolling_codes);
  RUN_TEST(test_groups_source_address);
  RUN_TEST(test_groups_single_group);
  RUN_TEST(test_groups_multiple_groups);
  RUN_TEST(test_unicast_source_address);
  RUN_TEST(test_unicast_destination_address);
  RUN_TEST(test_broadcast_source_address);
  RUN_TEST(test_parse_activate_scene);
  RUN_TEST(test_parse_store_scene);
  RUN_TEST(test_parse_fields);
  RUN_TEST(test_parse_fields_data_uint16);
  RUN_TEST(test_parse_fields_data_uint8);
  RUN_TEST(test_parse_fields_multiple_groups);
  RUN_TEST(test_physical_source_from_repeater);
  RUN_TEST(test_physical_source_from_hub);
  RUN_TEST(test_physical_source_from_remote);
  RUN_TEST(test_is_same_command_matching_direct_and_repeated);
  RUN_TEST(test_is_same_command_different_rolling_code);
  RUN_TEST(test_is_same_command_different_command);
  RUN_TEST(test_is_same_command_different_destination);
  RUN_TEST(test_parse_sender_type);
  return UNITY_END();
}

/**
 * For Arduino framework
 */

void setup()
{
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}
void loop() {}

int main(void)
{
  return runUnityTests();
}