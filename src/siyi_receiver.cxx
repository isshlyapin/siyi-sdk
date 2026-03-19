#include <memory>
#include <vector>
#include <expected>

#include "siyi-sdk/mydef.hxx"
#include "siyi-sdk/helpers.hxx"
#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/itransport.hxx"
#include "siyi-sdk/siyi_receiver.hxx"

namespace siyi {

SiyiReceiver::SiyiReceiver(std::shared_ptr<ITransport> transport)
: transport_(std::move(transport)) {}

std::expected<std::unique_ptr<protocol::SiyiFrame>, protocol::DecodeFrameError> 
SiyiReceiver::receive() {
  const auto popNextNewerPendingFrame = [this]()
      -> std::expected<std::unique_ptr<protocol::SiyiFrame>, protocol::DecodeFrameError> {
    bool hadPendingFrames = false;

    while (!pendingFrames_.empty()) {
      hadPendingFrames = true;

      auto frame = std::move(pendingFrames_.front());
      pendingFrames_.pop_front();

      if (!seqNewer(frame->seq, seq_)) {
        continue;
      }

      seq_ = frame->seq;
      return frame;
    }

    if (hadPendingFrames) {
      return std::unexpected(protocol::DecodeFrameError::OldFrame);
    }

    return std::unexpected(protocol::DecodeFrameError::NoFrame);
  };

  auto pendingFrame = popNextNewerPendingFrame();
  if (pendingFrame) {
    return std::move(pendingFrame.value());
  }

  auto result = transport_->receive();
  if (!result) {
    return std::unexpected(protocol::DecodeFrameError::NoFrame);
  }
  
  // dbgs << "Received datagram of size " << result.value()->buffer.size() << "\n";
  // print_datagram(*(result.value()));
  
  auto decodeResult = decodeDatagramFrames(std::move(result.value()));
  if (!decodeResult) {
    return std::unexpected(decodeResult.error());
  }

  return popNextNewerPendingFrame();
}

std::expected<void, protocol::DecodeFrameError>
SiyiReceiver::decodeDatagramFrames(std::unique_ptr<DataGram> datagram) {
  const auto& buffer = datagram->buffer;
  if (buffer.size() < protocol::MIN_FRAME_SIZE) {
    return std::unexpected(protocol::DecodeFrameError::TooShort);
  }

  size_t offset = 0;

  while (offset < buffer.size()) {
    const size_t remaining = buffer.size() - offset;
    if (remaining < protocol::MIN_FRAME_SIZE) {
      return std::unexpected(protocol::DecodeFrameError::TooShort);
    }

    const uint16_t inputStx = utility::read_u16_le(buffer, offset);
    if (inputStx != protocol::STX) {
      return std::unexpected(protocol::DecodeFrameError::BadStx);
    }

    const uint16_t dataLen = utility::read_u16_le(buffer, offset + 3);
    const size_t frameSize = protocol::MIN_FRAME_SIZE + static_cast<size_t>(dataLen);
    if (remaining < frameSize) {
      return std::unexpected(protocol::DecodeFrameError::LengthMismatch);
    }

    std::vector<uint8_t> frameBuffer(buffer.begin() + static_cast<std::ptrdiff_t>(offset),
                                     buffer.begin() + static_cast<std::ptrdiff_t>(offset + frameSize));

    auto frame = protocol::decode(std::make_unique<DataGram>(DataGram{std::move(frameBuffer)}));
    if (!frame) {
      return std::unexpected(frame.error());
    }

    pendingFrames_.push_back(std::move(frame.value()));
    offset += frameSize;
  }

  return {};
}

bool SiyiReceiver::seqNewer(uint16_t newSeq, uint16_t oldSeq) {
  return static_cast<int16_t>(newSeq - oldSeq) > 0;
}

} // namespace siyi