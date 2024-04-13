#ifndef HEX_HELPER_H
#define HEX_HELPER_H

#include <cstring>  // for strlen

uint8_t* hex_string_to_array(const char* str) {
  size_t len = strlen(str);
  if (len % 2 != 0) {
    // Cannot handle odd string length
    return nullptr;
  }
  size_t data_len = len / 2;

  uint8_t* data = new uint8_t[data_len];

  // Convert each hexadecimal character pair to a byte
  for (size_t i = 0; i < data_len; ++i) {
    uint8_t high_nibble = str[i * 2] >= 'A' ? (str[i * 2] - 'A' + 10) : (str[i * 2] - '0');
    uint8_t low_nibble = str[i * 2 + 1] >= 'A' ? (str[i * 2 + 1] - 'A' + 10) : (str[i * 2 + 1] - '0');
    
    // Combine the nibbles into a byte
    data[i] = (high_nibble << 4) | low_nibble;
  }

  // Return the pointer to the allocated data array
  return data;
}

#endif // HEX_HELPER_H