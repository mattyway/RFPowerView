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

void test_broadcast_source_address()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<GroupsHeader>(packet.header);

  TEST_ASSERT_EQUAL_HEX16(0x369E, header.source);

  delete[] packet_data;
}

void test_broadcast_single_group()
{
  const uint8_t* packet_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525300");

  Packet packet;

  run_parse_test(packet_data, packet);

  auto header = std::get<GroupsHeader>(packet.header);

  TEST_ASSERT_EQUAL_MESSAGE(1, header.groups.size(), "Group count");
  TEST_ASSERT_EQUAL_INT8_MESSAGE(4, header.groups[0], "Group at offset 0");

  delete[] packet_data;
}

void test_broadcast_multiple_groups()
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

int runUnityTests(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_parse_stop);
  RUN_TEST(test_parse_close);
  RUN_TEST(test_parse_open);
  RUN_TEST(test_parse_rolling_codes);
  RUN_TEST(test_broadcast_source_address);
  RUN_TEST(test_broadcast_single_group);
  RUN_TEST(test_broadcast_multiple_groups);
  RUN_TEST(test_unicast_source_address);
  RUN_TEST(test_unicast_destination_address);
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