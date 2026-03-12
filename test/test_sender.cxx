#include <memory>
#include <vector>
#include <cstdint>

#include <gtest/gtest.h>
#include <siyi-sdk/protocol.hxx>
#include <siyi-sdk/siyi_sender.hxx>

#include "fake_transport.hxx"

using namespace siyi;
using namespace siyi::protocol;
using namespace siyi::testing;

class SiyiSenderTest : public ::testing::Test {
protected:
  void SetUp() override {
    transport_ = std::make_shared<FakeTransport>();
    sender_ = std::make_unique<SiyiSender>(transport_);
  }

  std::shared_ptr<FakeTransport> transport_;
  std::unique_ptr<SiyiSender> sender_;
};

// ============================================================
// Базовая отправка
// ============================================================

TEST_F(SiyiSenderTest, SendFrameNoData) {
  auto frame = std::make_unique<SiyiFrame>();
  frame->ctrl = SiyiFrameCtrl::Request;
  frame->cmd  = SiyiFrameCmd::RequestGimbalCameraFirmwareVersion;

  ASSERT_TRUE(sender_->send(std::move(frame)));
  ASSERT_EQ(transport_->sentCount(), 1u);

  auto dg = transport_->popSent();
  ASSERT_NE(dg, nullptr);
  // Минимальный размер: 10 байт (stx + ctrl + data_len + seq + cmd + crc)
  EXPECT_GE(dg->buffer.size(), MIN_FRAME_SIZE);

  // Проверяем STX
  EXPECT_EQ(dg->buffer[0], 0x55);
  EXPECT_EQ(dg->buffer[1], 0x66);
  // cmd = 0x01
  EXPECT_EQ(dg->buffer[7], 0x01);
}

TEST_F(SiyiSenderTest, SendFrameWithData) {
  auto frame = std::make_unique<SiyiFrame>();
  frame->ctrl = SiyiFrameCtrl::Request;
  frame->cmd  = SiyiFrameCmd::SetGimbalRotation;
  frame->data = {0x64, 0x64};

  ASSERT_TRUE(sender_->send(std::move(frame)));
  ASSERT_EQ(transport_->sentCount(), 1u);

  auto dg = transport_->popSent();
  ASSERT_NE(dg, nullptr);
  EXPECT_EQ(dg->buffer.size(), MIN_FRAME_SIZE + 2u);
  // data_len = 2 (LE)
  EXPECT_EQ(dg->buffer[3], 0x02);
  EXPECT_EQ(dg->buffer[4], 0x00);
  // data
  EXPECT_EQ(dg->buffer[8], 0x64);
  EXPECT_EQ(dg->buffer[9], 0x64);
}

// ============================================================
// Автоинкремент seq
// ============================================================

TEST_F(SiyiSenderTest, SeqAutoIncrements) {
  for (int i = 0; i < 5; ++i) {
    auto frame = std::make_unique<SiyiFrame>();
    frame->ctrl = SiyiFrameCtrl::Request;
    frame->cmd  = SiyiFrameCmd::RequestGimbalAttitude;
    ASSERT_TRUE(sender_->send(std::move(frame)));
  }

  ASSERT_EQ(transport_->sentCount(), 5u);

  for (uint16_t i = 0; i < 5; ++i) {
    auto dg = transport_->popSent();
    ASSERT_NE(dg, nullptr);
    // seq хранится в байтах 5-6 (LE)
    uint16_t seq = dg->buffer[5] | (dg->buffer[6] << 8);
    EXPECT_EQ(seq, i);
  }
}

// ============================================================
// Ошибка транспорта
// ============================================================

TEST_F(SiyiSenderTest, SendFailsOnTransportError) {
  transport_->sendResult = TransportSendError::SendTimeout;

  auto frame = std::make_unique<SiyiFrame>();
  frame->ctrl = SiyiFrameCtrl::Request;
  frame->cmd  = SiyiFrameCmd::SetCenter;
  frame->data = {0x01};

  EXPECT_FALSE(sender_->send(std::move(frame)));
  EXPECT_EQ(transport_->sentCount(), 0u);
}

TEST_F(SiyiSenderTest, SendFailsNotOpen) {
  transport_->sendResult = TransportSendError::NotOpen;

  auto frame = std::make_unique<SiyiFrame>();
  frame->ctrl = SiyiFrameCtrl::Request;
  frame->cmd  = SiyiFrameCmd::RequestGimbalCameraHardwareID;

  EXPECT_FALSE(sender_->send(std::move(frame)));
}

// ============================================================
// Round-trip: отправить, декодировать обратно
// ============================================================

TEST_F(SiyiSenderTest, SentDatagramDecodesCorrectly) {
  auto frame = std::make_unique<SiyiFrame>();
  frame->ctrl = SiyiFrameCtrl::Response;
  frame->cmd  = SiyiFrameCmd::RequestDataStream;
  frame->data = {static_cast<uint8_t>(0x04)};

  ASSERT_TRUE(sender_->send(std::move(frame)));

  auto dg = transport_->popSent();
  auto decoded = decode(std::move(dg));
  ASSERT_TRUE(decoded.has_value());

  auto& f = *decoded.value();
  EXPECT_EQ(f.ctrl, SiyiFrameCtrl::Response);
  EXPECT_EQ(f.seq, 0u);
  EXPECT_EQ(f.cmd, SiyiFrameCmd::RequestDataStream);
  ASSERT_EQ(f.data.size(), 1u);
  EXPECT_EQ(f.data[0], 0x04);
}

// ============================================================
// Несколько фреймов подряд
// ============================================================

TEST_F(SiyiSenderTest, MultipleSendsAccumulate) {
  for (int i = 0; i < 3; ++i) {
    auto frame = std::make_unique<SiyiFrame>();
    frame->ctrl = SiyiFrameCtrl::Request;
    frame->cmd  = SiyiFrameCmd::RequestGimbalCameraPresentWorkingMode;
    ASSERT_TRUE(sender_->send(std::move(frame)));
  }

  EXPECT_EQ(transport_->sentCount(), 3u);

  // Забираем все и проверяем что cmd одинаков
  for (int i = 0; i < 3; ++i) {
    auto dg = transport_->popSent();
    ASSERT_NE(dg, nullptr);
    EXPECT_EQ(dg->buffer[7], static_cast<uint8_t>(SiyiFrameCmd::RequestGimbalCameraPresentWorkingMode));
  }
  EXPECT_EQ(transport_->sentCount(), 0u);
}
