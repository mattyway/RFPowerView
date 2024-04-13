#include <unity.h>
#include <Arduino.h>
#include "../hex_helper.h"
#include "PacketCRC.h"

void setUp()
{
}

void tearDown()
{
}

void run_calculate_test(const uint8_t *packet_data, const uint16_t expected_checksum)
{
  PacketCRC packetCRC;

  uint16_t actual_checksum = packetCRC.calculate(packet_data);

  TEST_ASSERT_EQUAL_UINT16(expected_checksum, actual_checksum);
}

void test_calculate_withDataLengthOf15()
{
  const uint8_t* packet_data = hex_string_to_array("C00F0005A1FFFF00008604FF000053471B");
  const uint16_t expected_checksum = 0x446B;

  run_calculate_test(packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf17()
{
  const uint8_t* packet_data = hex_string_to_array("C0110005D7FFFF369E8606B30400369E525300");
  const uint16_t expected_checksum = 0xE04C;

  run_calculate_test(packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf19()
{
  const uint8_t* packet_data = hex_string_to_array("C0130005DDFFFF00008605364EF100003F5A023F42");
  const uint16_t expected_checksum = 0x9BD3;

  run_calculate_test(packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf20()
{
  const uint8_t* packet_data = hex_string_to_array("C014100559FFFF4EF18605C200004EF1215A0321429C");
  const uint16_t expected_checksum = 0x4A8B;

  run_calculate_test(packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf21()
{
  const uint8_t* packet_data = hex_string_to_array("C0151005EAFFFF4EF186052400004EF121464E98070801");
  const uint16_t expected_checksum = 0x9887;

  run_calculate_test(packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf25()
{
  const uint8_t* packet_data = hex_string_to_array("C019000547FFFF000086053D4EF100003F5A023F50023F4D023F54");
  const uint16_t expected_checksum = 0x8318;

  run_calculate_test(packet_data, expected_checksum);

  delete[] packet_data;
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

int main(void)
{
  return runUnityTests();
}