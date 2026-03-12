#include <memory>
#include <vector>
#include <cstdint>

#include <gtest/gtest.h>
#include <siyi-sdk/protocol.hxx>
#include <siyi-sdk/siyi_receiver.hxx>

#include "fake_transport.hxx"

using namespace siyi;
using namespace siyi::protocol;
using namespace siyi::testing;

class SiyiReceiverTest : public ::testing::Test {
protected:
  void SetUp() override {
    transport_ = std::make_shared<FakeTransport>();
    receiver_ = std::make_unique<SiyiReceiver>(transport_);
  }

  /// Создаёт закодированную датаграмму из параметров фрейма.
  static std::unique_ptr<DataGram> encodedDatagram(
      SiyiFrameCtrl ctrl, uint16_t seq, SiyiFrameCmd cmd,
      std::vector<uint8_t> data = {}) {
    auto frame = std::make_unique<SiyiFrame>();
    frame->ctrl = ctrl;
    frame->seq  = seq;
    frame->cmd  = cmd;
    frame->data = std::move(data);
    return encode(std::move(frame));
  }

  std::shared_ptr<FakeTransport> transport_;
  std::unique_ptr<SiyiReceiver> receiver_;
};

// ============================================================
// Успешный приём
// ============================================================

TEST_F(SiyiReceiverTest, ReceiveValidFrame) {
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 1,
                      SiyiFrameCmd::RequestGimbalCameraFirmwareVersion));

  auto result = receiver_->receive();
  ASSERT_TRUE(result.has_value());

  auto& frame = *result.value();
  EXPECT_EQ(frame.ctrl, SiyiFrameCtrl::Response);
  EXPECT_EQ(frame.seq, 1u);
  EXPECT_EQ(frame.cmd, SiyiFrameCmd::RequestGimbalCameraFirmwareVersion);
  EXPECT_TRUE(frame.data.empty());
}

TEST_F(SiyiReceiverTest, ReceiveValidFrameWithData) {
  std::vector<uint8_t> payload = {0x10, 0x20, 0x30};
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 1,
                      SiyiFrameCmd::RequestDataStream, payload));

  auto result = receiver_->receive();
  ASSERT_TRUE(result.has_value());

  auto& frame = *result.value();
  EXPECT_EQ(frame.cmd, SiyiFrameCmd::RequestDataStream);
  EXPECT_EQ(frame.data, payload);
}

// ============================================================
// Последовательный приём с возрастающим seq
// ============================================================

TEST_F(SiyiReceiverTest, ReceiveMultipleFramesIncreasingSeq) {
  for (uint16_t seq = 1; seq <= 5; ++seq) {
    transport_->pushReceive(
        encodedDatagram(SiyiFrameCtrl::Response, seq,
                        SiyiFrameCmd::RequestGimbalAttitude));
  }

  for (uint16_t seq = 1; seq <= 5; ++seq) {
    auto result = receiver_->receive();
    ASSERT_TRUE(result.has_value()) << "Failed for seq=" << seq;
    EXPECT_EQ(result.value()->seq, seq);
  }
}

// ============================================================
// Фильтрация старых (OldFrame)
// ============================================================

TEST_F(SiyiReceiverTest, OldFrameIsRejected) {
  // Принимаем фрейм с seq=5
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 5,
                      SiyiFrameCmd::RequestGimbalAttitude));
  auto result = receiver_->receive();
  ASSERT_TRUE(result.has_value());

  // Теперь присылаем фрейм с seq=3 — старше текущего
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 3,
                      SiyiFrameCmd::RequestGimbalAttitude));
  auto old = receiver_->receive();
  ASSERT_FALSE(old.has_value());
  EXPECT_EQ(old.error(), DecodeFrameError::OldFrame);
}

TEST_F(SiyiReceiverTest, SameSeqIsRejected) {
  // Принимаем фрейм с seq=10
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 10,
                      SiyiFrameCmd::SetCenter));
  ASSERT_TRUE(receiver_->receive().has_value());

  // Тот же seq=10 — тоже считается старым
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 10,
                      SiyiFrameCmd::SetCenter));
  auto dup = receiver_->receive();
  ASSERT_FALSE(dup.has_value());
  EXPECT_EQ(dup.error(), DecodeFrameError::OldFrame);
}

TEST_F(SiyiReceiverTest, NewerSeqAfterOldRejections) {
  // seq=5 — OK
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 5,
                      SiyiFrameCmd::RequestGimbalAttitude));
  ASSERT_TRUE(receiver_->receive().has_value());

  // seq=2 — Old
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 2,
                      SiyiFrameCmd::RequestGimbalAttitude));
  EXPECT_FALSE(receiver_->receive().has_value());

  // seq=6 — OK (новее, чем 5)
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 6,
                      SiyiFrameCmd::RequestGimbalAttitude));
  auto result = receiver_->receive();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value()->seq, 6u);
}

// ============================================================
// Обёртка вокруг seq (wraparound)
// ============================================================

TEST_F(SiyiReceiverTest, SeqWraparound) {
  // Постепенно наращиваем seq до границы переполнения uint16_t
  // seqNewer(new, old) = int16_t(new - old) > 0
  // Максимальный «прыжок вперёд» — 0x7FFF

  // seq=1 (новее начального 0)
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 1,
                      SiyiFrameCmd::RequestGimbalAttitude));
  ASSERT_TRUE(receiver_->receive().has_value());

  // seq=0x7FFF (прыжок на 0x7FFE — валидный)
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 0x7FFF,
                      SiyiFrameCmd::RequestGimbalAttitude));
  ASSERT_TRUE(receiver_->receive().has_value());

  // seq=0xFFFE (прыжок на 0x7FFF — максимально допустимый)
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 0xFFFE,
                      SiyiFrameCmd::RequestGimbalAttitude));
  ASSERT_TRUE(receiver_->receive().has_value());

  // seq=0xFFFF
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 0xFFFF,
                      SiyiFrameCmd::RequestGimbalAttitude));
  ASSERT_TRUE(receiver_->receive().has_value());

  // seq=0 после переполнения:  int16_t(0 - 0xFFFF) = int16_t(1) > 0 → OK
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 0,
                      SiyiFrameCmd::RequestGimbalAttitude));
  auto result = receiver_->receive();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value()->seq, 0u);
}

// ============================================================
// Ошибки транспорта → NoFrame
// ============================================================

TEST_F(SiyiReceiverTest, TransportReceiveTimeout) {
  // Очередь пуста → FakeTransport вернёт ReceiveTimeout
  auto result = receiver_->receive();
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::NoFrame);
}

TEST_F(SiyiReceiverTest, TransportReceiveFailed) {
  transport_->receiveResult = TransportReceiveError::ReceiveFailed;

  auto result = receiver_->receive();
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::NoFrame);
}

TEST_F(SiyiReceiverTest, TransportNotOpen) {
  transport_->receiveResult = TransportReceiveError::NotOpen;

  auto result = receiver_->receive();
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::NoFrame);
}

// ============================================================
// Повреждённые датаграммы
// ============================================================

TEST_F(SiyiReceiverTest, CorruptedDatagram_TooShort) {
  // Слишком короткий буфер
  transport_->pushReceive({0x55, 0x66, 0x01});

  auto result = receiver_->receive();
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::TooShort);
}

TEST_F(SiyiReceiverTest, CorruptedDatagram_BadStx) {
  transport_->pushReceive({0xAA, 0xBB, 0x01, 0x00, 0x00,
                           0x01, 0x00, 0x19, 0x00, 0x00});

  auto result = receiver_->receive();
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::BadStx);
}

TEST_F(SiyiReceiverTest, CorruptedDatagram_BadCrc) {
  // Берём валидную датаграмму и портим CRC
  auto dg = encodedDatagram(SiyiFrameCtrl::Response, 1,
                            SiyiFrameCmd::RequestGimbalAttitude);
  // Портим последний байт (часть CRC)
  dg->buffer.back() ^= 0xFF;
  transport_->pushReceive(std::move(dg));

  auto result = receiver_->receive();
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::CrcMismatch);
}

// ============================================================
// Микс: ошибочная датаграмма не мешает принять следующую
// ============================================================

TEST_F(SiyiReceiverTest, RecoveryAfterCorruptedFrame) {
  // Первая — испорченная
  transport_->pushReceive({0x55, 0x66, 0x01, 0x00, 0x00,
                           0x01, 0x00, 0x19, 0x00, 0x00});

  auto bad = receiver_->receive();
  ASSERT_FALSE(bad.has_value());

  // Вторая — корректная
  transport_->pushReceive(
      encodedDatagram(SiyiFrameCtrl::Response, 1,
                      SiyiFrameCmd::RequestGimbalCameraHardwareID));

  auto good = receiver_->receive();
  ASSERT_TRUE(good.has_value());
  EXPECT_EQ(good.value()->cmd, SiyiFrameCmd::RequestGimbalCameraHardwareID);
}
