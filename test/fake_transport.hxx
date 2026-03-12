#pragma once

#include <deque>
#include <memory>
#include <vector>
#include <cstdint>
#include <expected>
#include <optional>

#include <siyi-sdk/itransport.hxx>

namespace siyi::testing {

/// FakeTransport — подмена ITransport для unit-тестов.
///
/// - `pushReceive(datagram)` — кладёт датаграмму в очередь на получение.
/// - `popSent()` — забирает последнюю отправленную датаграмму.
/// - Можно настроить ошибки, возвращаемые `open()`, `close()`, `send()`, `receive()`.
class FakeTransport : public ITransport {
public:
  // ---------- конфигурация ошибок ----------
  TransportOpenError    openResult    = TransportOpenError::Ok;
  TransportCloseError   closeResult   = TransportCloseError::Ok;
  TransportSendError    sendResult    = TransportSendError::Ok;
  TransportReceiveError receiveResult = TransportReceiveError::Ok;

  // ---------- ITransport ----------
  TransportOpenError open() override { return openResult; }

  TransportCloseError close() override { return closeResult; }

  TransportSendError send(std::unique_ptr<DataGram> data) override {
    if (sendResult != TransportSendError::Ok) {
      return sendResult;
    }
    sentDatagrams_.push_back(std::move(data));
    return TransportSendError::Ok;
  }

  std::expected<std::unique_ptr<DataGram>, TransportReceiveError> receive() override {
    if (receiveResult != TransportReceiveError::Ok) {
      return std::unexpected(receiveResult);
    }
    if (receiveQueue_.empty()) {
      return std::unexpected(TransportReceiveError::ReceiveTimeout);
    }
    auto dg = std::move(receiveQueue_.front());
    receiveQueue_.pop_front();
    return dg;
  }

  // ---------- тестовый API ----------

  /// Добавить датаграмму, которую вернёт следующий `receive()`.
  void pushReceive(std::unique_ptr<DataGram> datagram) {
    receiveQueue_.push_back(std::move(datagram));
  }

  /// Добавить «сырой» буфер в очередь на получение.
  void pushReceive(std::vector<uint8_t> buffer) {
    pushReceive(std::make_unique<DataGram>(std::move(buffer)));
  }

  /// Количество датаграмм, отправленных через `send()`.
  [[nodiscard]] size_t sentCount() const { return sentDatagrams_.size(); }

  /// Забрать первую отправленную датаграмму (FIFO).
  std::unique_ptr<DataGram> popSent() {
    if (sentDatagrams_.empty()) return nullptr;
    auto dg = std::move(sentDatagrams_.front());
    sentDatagrams_.pop_front();
    return dg;
  }

  /// Прочитать содержимое i-й отправленной датаграммы (не забирая).
  [[nodiscard]] const DataGram* peekSent(size_t index) const {
    if (index >= sentDatagrams_.size()) return nullptr;
    return sentDatagrams_[index].get();
  }

  /// Очистить историю отправок.
  void clearSent() { sentDatagrams_.clear(); }

  /// Очистить очередь приёма.
  void clearReceive() { receiveQueue_.clear(); }

private:
  std::deque<std::unique_ptr<DataGram>> receiveQueue_;
  std::deque<std::unique_ptr<DataGram>> sentDatagrams_;
};

} // namespace siyi::testing
