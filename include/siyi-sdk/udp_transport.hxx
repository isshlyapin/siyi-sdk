#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <expected>
#include <string_view>

#include <netdb.h>

#include "siyi-sdk/itransport.hxx"

namespace siyi {

constexpr const int SEND_TIMEOUT_MS = 300;
constexpr const int RECV_TIMEOUT_MS = 300;
constexpr const size_t MAX_DATAGRAM_SIZE = 2048;

class UdpTransport : public ITransport {
private:
  struct Options {
    uint16_t port{};
    std::string address;
  };

public:
  UdpTransport(std::string_view address, uint16_t port);

  ~UdpTransport() override;

  UdpTransport(const UdpTransport&) = delete;
  UdpTransport& operator=(const UdpTransport&) = delete;

  UdpTransport(UdpTransport&& other) noexcept;
  UdpTransport& operator=(UdpTransport&& other) noexcept;

  TransportOpenError open() override;

  TransportCloseError close() override;

  TransportSendError send(std::unique_ptr<DataGram> data) override;

  std::expected<std::unique_ptr<DataGram>, TransportReceiveError> receive() override;

private:
  static TransportOpenError apply_socket_timeouts(int fd);

  [[nodiscard]] std::expected<addrinfo*, TransportOpenError> getRemoteAddressInfo() const;


  static std::expected<int, TransportOpenError> createAndConnectSocket(const addrinfo* addrInfo);

  int fd_{-1};
  Options options_;
  bool is_open_{false};
};

} // namespace siyi
