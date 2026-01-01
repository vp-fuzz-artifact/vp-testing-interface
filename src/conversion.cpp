#include "vp_test/conversion.hpp"
#include <limits>

namespace testing::conversion {

/* Convert 32-bit int into uint8_t buffer[4] (bytes array) */
void int32_to_bytes(uint32_t value, uint8_t *buffer, std::size_t start) {
  buffer[start] = (uint8_t)((value >> 24) & 0xFF);
  buffer[start + 1] = (uint8_t)((value >> 16) & 0xFF);
  buffer[start + 2] = (uint8_t)((value >> 8) & 0xFF);
  buffer[start + 3] = (uint8_t)(value & 0xFF);
}

uint32_t bytes_to_int32(const uint8_t *buffer, std::size_t start) {
  return ((uint32_t)buffer[start] << 24) |
         ((uint32_t)buffer[start + 1] << 16) |
         ((uint32_t)buffer[start + 2] << 8) |
         ((uint32_t)buffer[start + 3]);
}

/* Convert 64-bit int into char buffer[8] (bytes array)*/
void int64_to_bytes(int64_t value, uint8_t *buffer, std::size_t start) {
  buffer[start] = (uint8_t)((value >> 56) & 0xFF); // MSB
  buffer[start + 1] = (uint8_t)((value >> 48) & 0xFF);
  buffer[start + 2] = (uint8_t)((value >> 40) & 0xFF);
  buffer[start + 3] = (uint8_t)((value >> 32) & 0xFF);
  buffer[start + 4] = (uint8_t)((value >> 24) & 0xFF);
  buffer[start + 5] = (uint8_t)((value >> 16) & 0xFF);
  buffer[start + 6] = (uint8_t)((value >> 8) & 0xFF);
  buffer[start + 7] = (uint8_t)(value & 0xFF); // LSB
}

int64_t bytes_to_int64(const uint8_t *buffer, std::size_t start) {
  return ((int64_t)buffer[start] << 56) |
         ((int64_t)buffer[start + 1] << 48) |
         ((int64_t)buffer[start + 2] << 40) |
         ((int64_t)buffer[start + 3] << 32) |
         ((int64_t)buffer[start + 4] << 24) |
         ((int64_t)buffer[start + 5] << 16) |
         ((int64_t)buffer[start + 6] << 8) |
         ((int64_t)buffer[start + 7]);
}

bool check_cast_to_uint32(uint64_t value) {
  return value <= std::numeric_limits<uint32_t>::max();
}

} // namespace testing::conversion
