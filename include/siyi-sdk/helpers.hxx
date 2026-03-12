#pragma once

#include <vector>
#include <cstdint>

namespace siyi::utility {

inline const auto write_i16 = [](std::vector<uint8_t>& buffer, size_t offset, int16_t value) {
  const uint16_t uvalue = static_cast<uint16_t>(value);
  buffer[offset] = uvalue & 0xff;
  buffer[offset + 1] = (uvalue >> 8) & 0xff;
};

inline const auto write_u16 = [](std::vector<uint8_t>& buffer, size_t offset, uint16_t value) {
  buffer[offset] = value >> 8;
  buffer[offset + 1] = value & 0xff;
};

inline const auto write_u16_le = [](std::vector<uint8_t>& buffer, size_t offset, uint16_t value) {
  buffer[offset] = value & 0xff;
  buffer[offset + 1] = (value >> 8) & 0xff;
};

inline const auto read_u16 = [](const std::vector<uint8_t>& buffer, size_t offset) -> uint16_t {
  return static_cast<uint16_t>(buffer[offset] << 8) | static_cast<uint16_t>(buffer[offset + 1]);
};

inline const auto read_u16_le = [](const std::vector<uint8_t>& buffer, size_t offset) -> uint16_t {
  return static_cast<uint16_t>(buffer[offset]) | static_cast<uint16_t>(buffer[offset + 1] << 8);
};

inline const auto read_i16 = [](const std::vector<uint8_t>& buffer, size_t offset) -> int16_t {
  return static_cast<int16_t>(buffer[offset] << 8) | (static_cast<int16_t>(buffer[offset + 1]));
};

inline const auto read_i16_le = [](const std::vector<uint8_t>& buffer, size_t offset) -> int16_t {
  return static_cast<int16_t>(buffer[offset]) | (static_cast<int16_t>(buffer[offset + 1]) << 8);
};

} // namespace siyi::utility