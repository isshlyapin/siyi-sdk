#include <algorithm>
#include <span>
#include <vector>
#include <memory>
#include <cstdint>
#include <expected>

#include "siyi-sdk/crc16.hxx"
#include "siyi-sdk/helpers.hxx"
#include "siyi-sdk/protocol.hxx"

namespace siyi::protocol {

std::expected<std::unique_ptr<SiyiFrame>, DecodeFrameError> 
  decode(std::unique_ptr<DataGram> datagram) {
  const auto& buffer = datagram->buffer;
  if (buffer.size() < MIN_FRAME_SIZE) {
    return std::unexpected(DecodeFrameError::TooShort);
  }

  SiyiFrame frame;
  const uint16_t inputStx = utility::read_u16_le(buffer, 0);
  if (inputStx != STX) {
    return std::unexpected(DecodeFrameError::BadStx);
  }

  // TODO(shlyapin): Validate ctrl
  frame.ctrl = static_cast<SiyiFrameCtrl>(buffer[2]);

  const uint16_t data_len = utility::read_u16_le(buffer, 3);
  if (buffer.size() != 8 + data_len + 2) {
    return std::unexpected(DecodeFrameError::LengthMismatch);
  }

  // TODO(shlyapin): Validate cmd
  frame.cmd = static_cast<SiyiFrameCmd>(buffer[7]);

  const uint16_t crc = utility::read_u16_le(buffer, 8 + data_len);
  const uint16_t calculatedCrc = crc::crc16(std::span(buffer.begin(), buffer.end() - 2));
  if (crc != calculatedCrc) {
    return std::unexpected(DecodeFrameError::CrcMismatch);
  }

  frame.seq = utility::read_u16_le(buffer, 5);
  
  frame.data.assign(buffer.begin() + 8, buffer.begin() + 8 + data_len);

  return std::make_unique<SiyiFrame>(std::move(frame));
}

std::unique_ptr<DataGram> encode(std::unique_ptr<SiyiFrame> frame) {
  const auto& frame_ref = *frame;

  std::vector<uint8_t> buffer;
  buffer.resize(8 + frame_ref.data.size() + 2);

  utility::write_u16_le(buffer, 0, STX);
  buffer[2] = static_cast<uint8_t>(frame_ref.ctrl);
  utility::write_u16_le(buffer, 3, static_cast<uint16_t>(frame_ref.data.size()));
  utility::write_u16_le(buffer, 5, frame_ref.seq);
  buffer[7] = static_cast<uint8_t>(frame_ref.cmd);
  std::ranges::copy(frame_ref.data, buffer.begin() + 8);
  utility::write_u16_le(buffer, 8 + frame_ref.data.size(), crc::crc16(std::span(buffer.begin(), buffer.end() - 2)));

  return std::make_unique<DataGram>(std::move(buffer));
}

} // namespace siyi::protocol
