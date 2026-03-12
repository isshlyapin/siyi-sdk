#pragma once

#include <memory>
#include <cstdint>

#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/itransport.hxx"

namespace siyi {

// TODO(shlyapin): Добавить поддержку многопоточной отправки сообщений
class SiyiSender {
public:
  explicit SiyiSender(std::shared_ptr<ITransport> transport);

  bool send(std::unique_ptr<protocol::SiyiFrame> frame);

private:
  std::shared_ptr<ITransport> transport_;
  uint16_t seq_{};
};

} // namespace siyi
