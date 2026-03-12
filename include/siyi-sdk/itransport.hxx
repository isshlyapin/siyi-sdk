#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <expected>

namespace siyi {

enum class TransportOpenError : uint8_t {
  Ok,
  Unknown,
  AlreadyOpen,
  SocketCreateFailed,
  SocketOptionFailed,
  AddressResolveFailed
};

enum class TransportCloseError : uint8_t {
  Ok,
  Unknown
};

enum class TransportSendError : uint8_t {
  Ok,
  Unknown,
  NotOpen,
  SendTimeout,
  SendSizeMismatch,
  SendMessageTooLarge
};

enum class TransportReceiveError : uint8_t {
  Ok,
  Unknown,
  NotOpen,
  ReceiveFailed,
  ReceiveTimeout
};

struct DataGram {
  std::vector<uint8_t> buffer;
};

class ITransport {
public:
  ITransport() = default;

  virtual ~ITransport() = default;
  
  ITransport(const ITransport &) = default;
  ITransport &operator=(const ITransport &) = default;
  
  ITransport(ITransport &&) = default;
  ITransport &operator=(ITransport &&) = default;

  virtual TransportOpenError open() = 0;

  virtual TransportCloseError close() = 0;

  virtual TransportSendError send(std::unique_ptr<DataGram> data) = 0;

  virtual std::expected<std::unique_ptr<DataGram>, TransportReceiveError> receive() = 0;
};

} // namespace siyi