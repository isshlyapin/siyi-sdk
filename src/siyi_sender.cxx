#include <memory>

#include "siyi-sdk/mydef.hxx"
#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/itransport.hxx"
#include "siyi-sdk/siyi_sender.hxx"

namespace siyi {

SiyiSender::SiyiSender(std::shared_ptr<ITransport> transport)
  : transport_(std::move(transport)) {}

bool SiyiSender::send(std::unique_ptr<protocol::SiyiFrame> frame) {
  frame->seq = seq_++;
  auto datagram = protocol::encode(std::move(frame));

  // dbgs << "Sending datagram of size " << datagram->buffer.size() << "\n";
  // print_datagram(*datagram);

  return transport_->send(std::move(datagram)) == TransportSendError::Ok;
}

} // namespace siyi