#include <memory>
#include <vector>
#include <utility>
#include <expected>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "siyi-sdk/mydef.hxx"
#include "siyi-sdk/itransport.hxx"
#include "siyi-sdk/udp_transport.hxx"

namespace siyi {

UdpTransport::UdpTransport(std::string_view address, uint16_t port)
  : options_{.port = port, .address = std::string(address)} {}

UdpTransport::~UdpTransport() {
  close();
}

UdpTransport::UdpTransport(UdpTransport&& other) noexcept
  : fd_(other.fd_), options_(std::move(other.options_)), is_open_(other.is_open_) {
  other.fd_ = -1;
  other.is_open_ = false;
}

UdpTransport& UdpTransport::operator=(UdpTransport&& other) noexcept {
  if (this != &other) {
    std::swap(fd_, other.fd_);
    std::swap(options_, other.options_);
    std::swap(is_open_, other.is_open_);
  }
  return *this;
}

TransportOpenError UdpTransport::open() {
  dbgs << "Opening UDP transport to " << options_.address << ":" << options_.port << "\n";
  if (is_open_) {
    dbgs << "\tError: UDP transport already open\n";
    return TransportOpenError::AlreadyOpen;
  }

  auto remoteAddressInfo = getRemoteAddressInfo();
  if (!remoteAddressInfo) {
    dbgs << "\tError: Failed to resolve address\n";
    return remoteAddressInfo.error();
  }

  auto socketResult = createAndConnectSocket(remoteAddressInfo.value());
  ::freeaddrinfo(remoteAddressInfo.value());

  if (!socketResult) {
    dbgs << "\tError: Failed to create and connect socket\n";
    return socketResult.error();
  }

  is_open_ = true;
  fd_ = socketResult.value();
  dbgs << "\tUDP transport opened successfully\n";
  return TransportOpenError::Ok;
}

std::expected<addrinfo*, TransportOpenError> UdpTransport::getRemoteAddressInfo() const {
  addrinfo hints{};
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  addrinfo* remote_result = nullptr;
  const auto remote_port_str = std::to_string(options_.port);

  const int gai_rc = ::getaddrinfo(
      options_.address.c_str(),
      remote_port_str.c_str(),
      &hints,
      &remote_result
  );

  if (gai_rc != 0) {
    return std::unexpected(TransportOpenError::AddressResolveFailed);
  }
  
  return remote_result;
}

std::expected<int, TransportOpenError> UdpTransport::createAndConnectSocket(const addrinfo* addrInfo) {
  for (const addrinfo* rp = addrInfo; rp != nullptr; rp = rp->ai_next) {
    int fd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd < 0) {
      continue;
    }

    auto err = apply_socket_timeouts(fd);
    if (err != TransportOpenError::Ok) {
      ::close(fd);
      continue;
    }

    if (::connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
      return fd;
    }

    ::close(fd);
  }
  return std::unexpected(TransportOpenError::SocketCreateFailed);
}

TransportCloseError UdpTransport::close() {
  if (is_open_) {
    ::close(fd_);
    fd_ = -1;
    is_open_ = false;
  }
  return TransportCloseError::Ok;
}

TransportSendError UdpTransport::send(std::unique_ptr<DataGram> data) {
  if (!is_open_) { 
    return TransportSendError::NotOpen; 
  }
  const auto& bytes = data->buffer;
  for (;;) {
    const ssize_t rc = ::send(fd_, bytes.data(), bytes.size(), 0);
    if (rc >= 0) {
      if (static_cast<std::size_t>(rc) != bytes.size()) {
        return TransportSendError::SendSizeMismatch;
      }
      return TransportSendError::Ok;
    }
    if (errno == EINTR) {
      continue;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return TransportSendError::SendTimeout;
    }
    if (errno == EMSGSIZE) {
      return TransportSendError::SendMessageTooLarge;
    }
    return TransportSendError::Unknown;
  }
}

std::expected<std::unique_ptr<DataGram>, TransportReceiveError> UdpTransport::receive() { 
  if (!is_open_) { 
    return std::unexpected(TransportReceiveError::NotOpen); 
  }  
  std::vector<uint8_t> buffer(MAX_DATAGRAM_SIZE);
  for (;;) { 
    const ssize_t rc = ::recv(fd_, buffer.data(), buffer.size(), 0);
    if (rc >= 0) { 
      buffer.resize(static_cast<std::size_t>(rc)); 
      return std::make_unique<DataGram>(std::move(buffer));
    } 
    if (errno == EINTR) { 
      continue; 
    }     
    if (errno == EAGAIN || errno == EWOULDBLOCK) { 
      return std::unexpected(TransportReceiveError::ReceiveTimeout); 
    }

    return std::unexpected(TransportReceiveError::ReceiveFailed); 
  } 
}

TransportOpenError UdpTransport::apply_socket_timeouts(int fd) {
  const auto make_timeval = [](int timeout_ms) -> timeval {
    timeval tv {};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return tv;
  };

  const timeval rcv_tv = make_timeval(RECV_TIMEOUT_MS);
  if (::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_tv, sizeof(rcv_tv)) < 0) {
    return TransportOpenError::SocketOptionFailed;
  }

  const timeval snd_tv = make_timeval(SEND_TIMEOUT_MS);
  if (::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &snd_tv, sizeof(snd_tv)) < 0) {
    return TransportOpenError::SocketOptionFailed;
  }

  return TransportOpenError::Ok;
}

} // namespace siyi
