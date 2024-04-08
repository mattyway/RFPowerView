#include <unity.h>
#include <Arduino.h>
#include "PacketCRC.h"

void run_calculate_test(const uint8_t *packet_data, const uint16_t expected_checksum)
{
  PacketCRC packetCRC;

  uint16_t actual_checksum = packetCRC.calculate(packet_data);

  TEST_ASSERT_EQUAL_UINT16(expected_checksum, actual_checksum);
}

void test_calculate_withDataLengthOf15()
{
  const uint8_t packet_data[] = {0xC0, 0x0F, 0x00, 0x05, 0xA1, 0xFF, 0xFF, 0x00, 0x00, 0x86, 0x04, 0xFF, 0x00, 0x00, 0x53, 0x47, 0x1B};
  const uint16_t expected_checksum = 0x446B;

  run_calculate_test(packet_data, expected_checksum);
}

void test_calculate_withDataLengthOf17()
{
  const uint8_t packet_data[] = {0xC0, 0x11, 0x00, 0x05, 0xD7, 0xFF, 0xFF, 0x36, 0x9E, 0x86, 0x06, 0xB3, 0x04, 0x00, 0x36, 0x9E, 0x52, 0x53, 0x00};
  const uint16_t expected_checksum = 0xE04C;

  run_calculate_test(packet_data, expected_checksum);
}

void test_calculate_withDataLengthOf19()
{
  const uint8_t packet_data[] = {0xC0, 0x13, 0x00, 0x05, 0xDD, 0xFF, 0xFF, 0x00, 0x00, 0x86, 0x05, 0x36, 0x4E, 0xF1, 0x00, 0x00, 0x3F, 0x5A, 0x02, 0x3F, 0x42};
  const uint16_t expected_checksum = 0x9BD3;

  run_calculate_test(packet_data, expected_checksum);
}

void test_calculate_withDataLengthOf20()
{
  const uint8_t packet_data[] = {0xC0, 0x14, 0x10, 0x05, 0x59, 0xFF, 0xFF, 0x4E, 0xF1, 0x86, 0x05, 0xC2, 0x00, 0x00, 0x4E, 0xF1, 0x21, 0x5A, 0x03, 0x21, 0x42, 0x9C};
  const uint16_t expected_checksum = 0x4A8B;

  run_calculate_test(packet_data, expected_checksum);
}

void test_calculate_withDataLengthOf21()
{
  const uint8_t packet_data[] = {0xC0, 0x15, 0x10, 0x05, 0xEA, 0xFF, 0xFF, 0x4E, 0xF1, 0x86, 0x05, 0x24, 0x00, 0x00, 0x4E, 0xF1, 0x21, 0x46, 0x4E, 0x98, 0x07, 0x08, 0x01};
  const uint16_t expected_checksum = 0x9887;

  run_calculate_test(packet_data, expected_checksum);
}

void test_calculate_withDataLengthOf25()
{
  const uint8_t packet_data[] = {0xC0, 0x19, 0x00, 0x05, 0x47, 0xFF, 0xFF, 0x00, 0x00, 0x86, 0x05, 0x3D, 0x4E, 0xF1, 0x00, 0x00, 0x3F, 0x5A, 0x02, 0x3F, 0x50, 0x02, 0x3F, 0x4D, 0x02, 0x3F, 0x54};
  const uint16_t expected_checksum = 0x8318;

  run_calculate_test(packet_data, expected_checksum);
}

int runUnityTests(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_calculate_withDataLengthOf15);
  RUN_TEST(test_calculate_withDataLengthOf17);
  RUN_TEST(test_calculate_withDataLengthOf19);
  RUN_TEST(test_calculate_withDataLengthOf20);
  RUN_TEST(test_calculate_withDataLengthOf21);
  RUN_TEST(test_calculate_withDataLengthOf25);
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