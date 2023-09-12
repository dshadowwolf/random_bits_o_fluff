#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

// cribbed from a description of how this is supposed to work
#define CRC32_POLYNOMIAL 0xEDB88320u

uint32_t crc32(unsigned char *message) {
  uint32_t crc = 0xFFFFFFFFu; // CRC is always initialized to all set bits

  for (int i = 0; i < strlen(message); i++) {
    crc ^= message[i];
    uint32_t mask;

    for (int j = 7; j >= 0; j--) { // once per bit!
      mask = -(crc & 1);                             // mask for just the low-order bit
      crc = (crc >> 1) ^ (CRC32_POLYNOMIAL & mask);  // add into the calculation
    }
  }

  return ~crc; // invert the value and return
}
