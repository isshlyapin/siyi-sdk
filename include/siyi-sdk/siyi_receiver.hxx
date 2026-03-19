#pragma once

#include <memory>
#include <deque>
#include <cstdint>
#include <expected>

#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/itransport.hxx"

namespace siyi {

class SiyiReceiver {
public:
  explicit SiyiReceiver(std::shared_ptr<ITransport> transport);

  std::expected<std::unique_ptr<protocol::SiyiFrame>, protocol::DecodeFrameError> 
    receive();

private:
  std::expected<void, protocol::DecodeFrameError>
    decodeDatagramFrames(std::unique_ptr<DataGram> datagram);

  static bool seqNewer(uint16_t newSeq, uint16_t oldSeq);

  std::shared_ptr<ITransport> transport_;
  std::deque<std::unique_ptr<protocol::SiyiFrame>> pendingFrames_;
  uint16_t seq_{};
};

} // namespace siyi
