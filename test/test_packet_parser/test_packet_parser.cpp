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

  TEST_ASSERT_TRUE(result);
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

int runUnityTests(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_parse_stop);
  RUN_TEST(test_parse_close);
  RUN_TEST(test_parse_open);
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