#include <unity.h>
#include <Arduino.h>
#include "../hex_helper.h"
#include "BufferFiller.h"

void setUp()
{
}

void tearDown()
{
}

void run_fill_test(const Packet &packet, uint8_t *packet_data)
{
  BufferFiller bufferFiller;

  bool result = bufferFiller.fill(packet_data, &packet);

  TEST_ASSERT_TRUE_MESSAGE(result, "Unable to fill packet_data");
}

void test_fill_groups_stop()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = GroupsHeader {};
  header.groups = std::vector<uint8_t>();
  header.groups.push_back(0x04);
  header.source = 0x369E;
  packet.header = header;
  packet.type = PacketType::STOP;
  packet.rollingCode1 = 0x6C;
  packet.rollingCode2 = 0x3C;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E5253003BF8");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 21);
  
  delete[] expected_data;
}

void test_fill_groups_close()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = GroupsHeader {};
  header.groups = std::vector<uint8_t>();
  header.groups.push_back(0x04);
  header.source = 0x369E;
  packet.header = header;
  packet.type = PacketType::CLOSE;
  packet.rollingCode1 = 0x6C;
  packet.rollingCode2 = 0x3C;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E524400E80D");

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 21);
  
  delete[] expected_data;
}

void test_fill_groups_open()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = GroupsHeader {};
  header.groups = std::vector<uint8_t>();
  header.groups.push_back(0x04);
  header.source = 0x369E;
  packet.header = header;
  packet.type = PacketType::OPEN;
  packet.rollingCode1 = 0x6C;
  packet.rollingCode2 = 0x3C;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C01100056CFFFF369E86063C0400369E525500B988");

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 21);

  delete[] expected_data;
}

void test_fill_activate_scene()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = BroadcastHeader {};
  header.source = 0x0000;
  packet.header = header;
  packet.type = PacketType::ACTIVATE_SCENE;
  auto parameters = ActivateSceneParameters {};
  parameters.sceneID = 0x1B;
  packet.parameters = parameters;
  packet.rollingCode1 = 0xA1;
  packet.rollingCode2 = 0xFF;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C00F0005A1FFFF00008604FF000053471B446B");

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 19);

  delete[] expected_data;
}

void test_fill_multiple_groups()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = GroupsHeader {};
  header.groups = std::vector<uint8_t>();
  header.groups.push_back(0x01);
  header.groups.push_back(0x02);
  header.groups.push_back(0x03);
  header.groups.push_back(0x05);
  header.source = 0x369E;
  packet.header = header;
  packet.type = PacketType::OPEN;
  packet.rollingCode1 = 0x6C;
  packet.rollingCode2 = 0x3C;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C01400056CFFFF369E86063C0102030500369E525500E050");

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 24);

  delete[] expected_data;
}

void test_fill_unicast_stop()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::STOP;
  packet.rollingCode1 = 0x4A;
  packet.rollingCode2 = 0xF2;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C01100054AFFFF00008605F24EF100005253003F1F");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 21);
  
  delete[] expected_data;
}

void test_fill_unicast_close()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::CLOSE;
  packet.rollingCode1 = 0x4A;
  packet.rollingCode2 = 0xF2;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C01100054AFFFF00008605F24EF10000524400ECEA");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 21);
  
  delete[] expected_data;
}

void test_fill_unicast_open()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::OPEN;
  packet.rollingCode1 = 0x4A;
  packet.rollingCode2 = 0xF2;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C01100054AFFFF00008605F24EF10000525500BD6F");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 21);
  
  delete[] expected_data;
}

void test_fill_fields_fetch()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::FIELD_COMMAND;
  auto parameters = FieldsParameters {};
  auto field1 = Field {};
  field1.identifier = 0x50;
  field1.type = FieldType::FETCH;
  field1.hasValue = false;
  parameters.fields.push_back(field1);
  auto field2 = Field {};
  field2.identifier = 0x4D;
  field2.type = FieldType::FETCH;
  field2.hasValue = false;
  parameters.fields.push_back(field2);
  auto field3 = Field {};
  field3.identifier = 0x54;
  field3.type = FieldType::FETCH;
  field3.hasValue = false;
  parameters.fields.push_back(field3);
  packet.parameters = parameters;
  packet.rollingCode1 = 0x92;
  packet.rollingCode2 = 0x4E;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C019000592FFFF000086054E4EF100003F5A023F50023F4D023F549D76");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 29);
  
  delete[] expected_data;
}

void test_fill_fields_set()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::FIELD_COMMAND;
  auto parameters = FieldsParameters {};
  auto field1 = Field {};
  field1.identifier = 0x50;
  field1.type = FieldType::SET;
  field1.hasValue = true;
  field1.value = (uint16_t)0xFFFF;
  parameters.fields.push_back(field1);
  packet.parameters = parameters;
  packet.rollingCode1 = 0x29;
  packet.rollingCode2 = 0xB9;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C015000529FFFF00008605B94EF100003F5A044050FFFF286B");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 25);
  
  delete[] expected_data;
}

int runUnityTests(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_fill_groups_stop);
  RUN_TEST(test_fill_groups_close);
  RUN_TEST(test_fill_groups_open);
  RUN_TEST(test_fill_activate_scene);
  RUN_TEST(test_fill_multiple_groups);
  RUN_TEST(test_fill_unicast_stop);
  RUN_TEST(test_fill_unicast_close);
  RUN_TEST(test_fill_unicast_open);
  RUN_TEST(test_fill_fields_fetch);
  RUN_TEST(test_fill_fields_set);
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