//*********************************************************//
//       CRC16 Coding & Decoding G(X) = X^16+X^12+X^5+1    //
//*********************************************************//

#pragma once

#include <span>
#include <cstdint>

namespace siyi::crc {

uint16_t crc16(std::span<const uint8_t> data);

} // namespace siyi