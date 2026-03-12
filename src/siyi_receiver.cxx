#include <memory>
#include <expected>

#include "siyi-sdk/mydef.hxx"
#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/itransport.hxx"
#include "siyi-sdk/siyi_receiver.hxx"

namespace siyi {

SiyiReceiver::SiyiReceiver(std::shared_ptr<ITransport> transport)
: transport_(std::move(transport)) {}

std::expected<std::unique_ptr<protocol::SiyiFrame>, protocol::DecodeFrameError> 
SiyiReceiver::receive() {
  auto result = transport_->receive();
  if (!result) {
    return std::unexpected(protocol::DecodeFrameError::NoFrame);
  }
  
  // dbgs << "Received datagram of size " << result.value()->buffer.size() << "\n";
  // print_datagram(*(result.value()));
  
  auto frame = protocol::decode(std::move(result.value()));
  if (!frame) {
    return std::unexpected(frame.error());
  }
  
  if (!seqNewer(frame.value()->seq, seq_)) {
    return std::unexpected(protocol::DecodeFrameError::OldFrame);
  }
  
  seq_ = frame.value()->seq;
  return std::move(frame.value());    
}

bool SiyiReceiver::seqNewer(uint16_t newSeq, uint16_t oldSeq) {
  return static_cast<int16_t>(newSeq - oldSeq) > 0;
}

} // namespace siyi