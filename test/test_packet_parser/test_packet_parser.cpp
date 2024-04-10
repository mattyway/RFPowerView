#include <unity.h>
#include <Arduino.h>
#include "PacketParser.h"

void run_parse_test(const uint8_t *packet_data, Packet& packet)
{
  PacketParser packetParser;

  bool result = packetParser.parsePacket(packet_data, packet);

  TEST_ASSERT_TRUE(result);
}

void test_parse_stop()
{
  const uint8_t packet_data[] = {0xC0, 0x11, 0x00, 0x05, 0x6C, 0xFF, 0xFF, 0x36, 0x9E, 0x86, 0x06, 0x3C, 0x04, 0x00, 0x36, 0x9E, 0x52, 0x53, 0x00};

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::STOP);
}

void test_parse_close()
{
  const uint8_t packet_data[] = {0xC0, 0x11, 0x00, 0x05, 0x6C, 0xFF, 0xFF, 0x36, 0x9E, 0x86, 0x06, 0x3C, 0x04, 0x00, 0x36, 0x9E, 0x52, 0x44, 0x00};

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::CLOSE);
}

void test_parse_open()
{
  const uint8_t packet_data[] = {0xC0, 0x11, 0x00, 0x05, 0x6C, 0xFF, 0xFF, 0x36, 0x9E, 0x86, 0x06, 0x3C, 0x04, 0x00, 0x36, 0x9E, 0x52, 0x55, 0x00};

  Packet packet;

  run_parse_test(packet_data, packet);

  TEST_ASSERT_TRUE(packet.type == PacketType::OPEN);
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
