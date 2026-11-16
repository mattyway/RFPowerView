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

void run_calculate_test(const uint16_t rf_id, const uint8_t *packet_data, const uint16_t expected_checksum)
{
  PacketCRC packetCRC(rf_id);

  uint16_t actual_checksum = packetCRC.calculate(packet_data);

  TEST_ASSERT_EQUAL_HEX16(expected_checksum, actual_checksum);
}

void test_calculate_withDataLengthOf15()
{
  const uint8_t* packet_data = hex_string_to_array("C00F0005A1FFFF00008604FF000053471B");
  const uint16_t expected_checksum = 0x446B;

  run_calculate_test(0x2EC8, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf17()
{
  const uint8_t* packet_data = hex_string_to_array("C0110005D7FFFF369E8606B30400369E525300");
  const uint16_t expected_checksum = 0xE04C;

  run_calculate_test(0x2EC8, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf19()
{
  const uint8_t* packet_data = hex_string_to_array("C0130005DDFFFF00008605364EF100003F5A023F42");
  const uint16_t expected_checksum = 0x9BD3;

  run_calculate_test(0x2EC8, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf20()
{
  const uint8_t* packet_data = hex_string_to_array("C014100559FFFF4EF18605C200004EF1215A0321429C");
  const uint16_t expected_checksum = 0x4A8B;

  run_calculate_test(0x2EC8, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf21()
{
  const uint8_t* packet_data = hex_string_to_array("C0151005EAFFFF4EF186052400004EF121464E98070801");
  const uint16_t expected_checksum = 0x9887;

  run_calculate_test(0x2EC8, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withDataLengthOf25()
{
  const uint8_t* packet_data = hex_string_to_array("C019000547FFFF000086053D4EF100003F5A023F50023F4D023F54");
  const uint16_t expected_checksum = 0x8318;

  run_calculate_test(0x2EC8, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withRfID0C43()
{
  const uint8_t* packet_data = hex_string_to_array("C0110005BFFFFF37868606C105003786525300");
  const uint16_t expected_checksum = 0xB6C4;

  run_calculate_test(0x0C43, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_calculate_withRfID1111()
{
  const uint8_t* packet_data = hex_string_to_array("C0110005A4FFFF5A0E86064E01005A0E524400");
  const uint16_t expected_checksum = 0x3E79;

  run_calculate_test(0x1111, packet_data, expected_checksum);

  delete[] packet_data;
}

void test_check_withMatchingRfID()
{
  const uint8_t* packet_2ec8 = hex_string_to_array("C0110005D7FFFF369E8606B30400369E525300E04C");
  const uint8_t* packet_0c43 = hex_string_to_array("C0110005BFFFFF37868606C105003786525300B6C4");
  const uint8_t* packet_1111 = hex_string_to_array("C0110005A4FFFF5A0E86064E01005A0E5244003E79");

  TEST_ASSERT_TRUE(PacketCRC(0x2EC8).check(packet_2ec8));
  TEST_ASSERT_TRUE(PacketCRC(0x0C43).check(packet_0c43));
  TEST_ASSERT_TRUE(PacketCRC(0x1111).check(packet_1111));

  delete[] packet_2ec8;
  delete[] packet_0c43;
  delete[] packet_1111;
}

void test_check_withDifferentRfID()
{
  const uint8_t* packet_data = hex_string_to_array("C0110005D7FFFF369E8606B30400369E525300E04C");

  TEST_ASSERT_FALSE(PacketCRC(0x0C43).check(packet_data));

  delete[] packet_data;
}

void test_check_withCorruptedChecksum()
{
  const uint8_t* packet_data = hex_string_to_array("C0110005D7FFFF369E8606B30400369E525300E04D");

  TEST_ASSERT_FALSE(PacketCRC(0x2EC8).check(packet_data));

  delete[] packet_data;
}

void test_check_withInvalidLength()
{
  const uint8_t* zero_length = hex_string_to_array("C0000005D7FFFF369E8606B30400369E525300E04C");
  const uint8_t* oversized = hex_string_to_array("C01D0005D7FFFF369E8606B30400369E525300E04C");

  TEST_ASSERT_FALSE(PacketCRC(0x2EC8).check(zero_length));
  TEST_ASSERT_FALSE(PacketCRC(0x2EC8).check(oversized));

  delete[] zero_length;
  delete[] oversized;
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
  RUN_TEST(test_calculate_withRfID0C43);
  RUN_TEST(test_calculate_withRfID1111);
  RUN_TEST(test_check_withMatchingRfID);
  RUN_TEST(test_check_withDifferentRfID);
  RUN_TEST(test_check_withCorruptedChecksum);
  RUN_TEST(test_check_withInvalidLength);
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