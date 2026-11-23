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
  BufferFiller bufferFiller(0x2EC8);

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

void test_fill_store_scene()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.destination = 0x4EF1;
  header.source = 0x0000;
  packet.header = header;
  packet.type = PacketType::STORE_SCENE;
  auto parameters = StoreSceneParameters {};
  parameters.sceneID = 0x1B;
  packet.parameters = parameters;
  packet.rollingCode1 = 0x29;
  packet.rollingCode2 = 0xB9;

  run_fill_test(packet, packet_data);

  TEST_ASSERT_EQUAL_HEX8(0xC0, packet_data[0]);
  TEST_ASSERT_EQUAL_HEX8(0x11, packet_data[1]);
  TEST_ASSERT_EQUAL_HEX8(0x53, packet_data[16]);
  TEST_ASSERT_EQUAL_HEX8(0x43, packet_data[17]);
  TEST_ASSERT_EQUAL_HEX8(0x1B, packet_data[18]);
  TEST_ASSERT_TRUE(PacketCRC(0x2EC8).check(packet_data));
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

void test_fill_properties_fetch()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::GET_PROPERTIES;
  auto parameters = PropertiesParameters {};
  auto prop1 = Property {};
  prop1.identifier = 0x50;
  prop1.type = PropertyType::FETCH;
  prop1.hasValue = false;
  parameters.properties.push_back(prop1);
  auto prop2 = Property {};
  prop2.identifier = 0x4D;
  prop2.type = PropertyType::FETCH;
  prop2.hasValue = false;
  parameters.properties.push_back(prop2);
  auto prop3 = Property {};
  prop3.identifier = 0x54;
  prop3.type = PropertyType::FETCH;
  prop3.hasValue = false;
  parameters.properties.push_back(prop3);
  packet.parameters = parameters;
  packet.rollingCode1 = 0x92;
  packet.rollingCode2 = 0x4E;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C019000592FFFF000086054E4EF100003F5A023F50023F4D023F549D76");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 29);
  
  delete[] expected_data;
}

void test_fill_properties_set()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::SET_PROPERTIES;
  auto parameters = PropertiesParameters {};
  auto prop1 = Property {};
  prop1.identifier = 0x50;
  prop1.type = PropertyType::SET;
  prop1.hasValue = true;
  prop1.value = (uint16_t)0xFFFF;
  parameters.properties.push_back(prop1);
  packet.parameters = parameters;
  packet.rollingCode1 = 0x29;
  packet.rollingCode2 = 0xB9;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C015000529FFFF00008605B94EF100003F5A044050FFFF286B");
  
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 25);
  
  delete[] expected_data;
}

void test_fill_repeated_packet()
{
  uint8_t packet_data[32];

  Packet packet;
  auto header = UnicastHeader {};
  header.source = 0x0000;
  header.destination = 0x4EF1;
  packet.header = header;
  packet.type = PacketType::GET_PROPERTIES;
  auto parameters = PropertiesParameters {};
  parameters.properties.push_back(Property{0x50, PropertyType::FETCH, false, std::monostate{}});
  parameters.properties.push_back(Property{0x4D, PropertyType::FETCH, false, std::monostate{}});
  parameters.properties.push_back(Property{0x54, PropertyType::FETCH, false, std::monostate{}});
  packet.parameters = parameters;
  packet.rollingCode1 = 0x92;
  packet.rollingCode2 = 0x4E;
  packet.physicalSource = 0x72CB;
  packet.repeated = true;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C019000592FFFF72CB85054E4EF100003F5A023F50023F4D023F54C9F3");

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 29);

  delete[] expected_data;
}

void test_fill_sender_type()
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
  packet.senderType = SenderType::SHADE;

  run_fill_test(packet, packet_data);

  TEST_ASSERT_EQUAL_HEX8(0x10, packet_data[2]);
}

void test_fill_firmware_query()
{
  uint8_t packet_data[32];

  Packet packet;
  packet.header = UnicastHeader{0x0000, 0x4EF1};
  packet.type = PacketType::GET_FIRMWARE;
  packet.parameters = FirmwareParameters{FirmwareTarget::BLIND_CONTROLLER};
  packet.rollingCode1 = 0xC1;
  packet.rollingCode2 = 0x6A;
  packet.repeated = false;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C0110005C1FFFF000086056A4EF100003F464EBA02");
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 21);
  delete[] expected_data;
}

void test_fill_firmware_report_blind()
{
  uint8_t packet_data[32];

  Packet packet;
  packet.header = UnicastHeader{0x4EF1, 0x0000};
  packet.physicalSource = 0x4EF1;
  packet.senderType = SenderType::SHADE;
  packet.type = PacketType::REPORT_FIRMWARE;
  FirmwareParameters fw;
  fw.target = FirmwareTarget::BLIND_CONTROLLER;
  fw.major = 1;
  fw.minor = 8;
  fw.build = 1944;
  packet.parameters = fw;
  packet.rollingCode1 = 0x94;
  packet.rollingCode2 = 0xAB;
  packet.repeated = false;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C015100594FFFF4EF18605AB00004EF121464E980708018DE7");
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 25);
  delete[] expected_data;
}

void test_fill_firmware_report_motor()
{
  uint8_t packet_data[32];

  Packet packet;
  packet.header = UnicastHeader{0x4EF1, 0x0000};
  packet.physicalSource = 0x4EF1;
  packet.senderType = SenderType::SHADE;
  packet.type = PacketType::REPORT_FIRMWARE;
  FirmwareParameters fw;
  fw.target = FirmwareTarget::MOTOR_CONTROLLER;
  fw.setAscii("2.00");
  TEST_ASSERT_EQUAL_UINT8(48, fw.major);
  TEST_ASSERT_EQUAL_UINT8(48, fw.minor);
  TEST_ASSERT_EQUAL_UINT16(11826, fw.build);
  packet.parameters = fw;
  packet.rollingCode1 = 0xA0;
  packet.rollingCode2 = 0xB7;
  packet.repeated = false;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C0151005A0FFFF4EF18605B700004EF1214643322E3030EF05");
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 25);
  delete[] expected_data;
}

void test_fill_compound_query_with_shade_type()
{
  uint8_t packet_data[32];

  Packet packet;
  packet.header = UnicastHeader{0x0000, 0x4EF1};
  packet.type = PacketType::GET_PROPERTIES;
  packet.rollingCode1 = 0xBA;
  packet.rollingCode2 = 0x63;
  PropertiesParameters properties;
  properties.properties.push_back(Property{0x50, PropertyType::FETCH, false, std::monostate{}});
  properties.properties.push_back(Property{0x4D, PropertyType::FETCH, false, std::monostate{}});
  properties.properties.push_back(Property{0x54, PropertyType::FETCH, false, std::monostate{}});
  packet.parameters = properties;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C0190005BAFFFF00008605634EF100003F5A023F50023F4D023F546F81");
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, packet_data, 29);
  delete[] expected_data;
}

void test_fill_report_properties()
{
  uint8_t packet_data[32];

  Packet packet;
  packet.header = UnicastHeader{0x4EF1, 0x0000};
  packet.physicalSource = 0x4EF1;
  packet.senderType = SenderType::SHADE;
  packet.type = PacketType::REPORT_PROPERTIES;
  PropertiesParameters properties;
  properties.properties.push_back(Property{0x50, PropertyType::VALUE, true, (uint16_t)0x0140});
  packet.parameters = properties;
  packet.rollingCode1 = 0xE0;
  packet.rollingCode2 = 0x1A;
  packet.repeated = false;

  run_fill_test(packet, packet_data);

  const uint8_t* expected_data = hex_string_to_array("C0151005E0FFFF4EF186051A00004EF1215A04215040016670");
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
  RUN_TEST(test_fill_store_scene);
  RUN_TEST(test_fill_multiple_groups);
  RUN_TEST(test_fill_unicast_stop);
  RUN_TEST(test_fill_unicast_close);
  RUN_TEST(test_fill_unicast_open);
  RUN_TEST(test_fill_properties_fetch);
  RUN_TEST(test_fill_properties_set);
  RUN_TEST(test_fill_repeated_packet);
  RUN_TEST(test_fill_sender_type);
  RUN_TEST(test_fill_firmware_query);
  RUN_TEST(test_fill_firmware_report_blind);
  RUN_TEST(test_fill_firmware_report_motor);
  RUN_TEST(test_fill_compound_query_with_shade_type);
  RUN_TEST(test_fill_report_properties);
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