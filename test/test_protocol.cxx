#include <vector>
#include <memory>
#include <cstdint>

#include <gtest/gtest.h>
#include <siyi-sdk/protocol.hxx>
#include <siyi-sdk/crc16.hxx>

using namespace siyi;
using namespace siyi::protocol;

// ============================================================
// Вспомогательные функции
// ============================================================

static std::unique_ptr<DataGram> makeDatagram(std::vector<uint8_t> buf) {
  return std::make_unique<DataGram>(std::move(buf));
}

static std::unique_ptr<SiyiFrame> makeFrame(
    SiyiFrameCtrl ctrl, uint16_t seq, SiyiFrameCmd cmd,
    std::vector<uint8_t> data = {}) {
  auto f = std::make_unique<SiyiFrame>();
  f->ctrl = ctrl;
  f->seq  = seq;
  f->cmd  = cmd;
  f->data = std::move(data);
  return f;
}

// ============================================================
// Decode: успешное декодирование
// ============================================================

// 55 66 01 00 00 00 00 19 5D 57
// ctrl=Response, data_len=0, seq=0, cmd=0x19, data={}
TEST(Decode, ValidFrameNoData) {
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x19, 0x5D, 0x57});
  auto result = decode(std::move(dg));
  ASSERT_TRUE(result.has_value());
  auto& frame = *result.value();
  EXPECT_EQ(frame.ctrl, SiyiFrameCtrl::Response);
  EXPECT_EQ(frame.seq, 0);
  EXPECT_EQ(frame.cmd, SiyiFrameCmd::RequestGimbalCameraPresentWorkingMode);
  EXPECT_TRUE(frame.data.empty());
}

// 55 66 01 01 00 00 00 05 01 8d 64
// ctrl=Response, data_len=1, seq=0, cmd=0x05, data={0x01}
TEST(Decode, ValidFrameWithOneByteData) {
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x01, 0x00,
                          0x00, 0x00, 0x05, 0x01, 0x8d, 0x64});
  auto result = decode(std::move(dg));
  ASSERT_TRUE(result.has_value());
  auto& frame = *result.value();
  EXPECT_EQ(frame.ctrl, SiyiFrameCtrl::Response);
  EXPECT_EQ(frame.seq, 0);
  EXPECT_EQ(static_cast<uint8_t>(frame.cmd), 0x05);
  ASSERT_EQ(frame.data.size(), 1u);
  EXPECT_EQ(frame.data[0], 0x01);
}

// 55 66 01 02 00 10 00 0f 04 05 6b 15
// ctrl=Response, data_len=2, seq=16, cmd=0x0f, data={0x04,0x05}
TEST(Decode, ValidFrameWithTwoBytesDataAndNonZeroSeq) {
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x02, 0x00,
                          0x10, 0x00, 0x0f, 0x04, 0x05, 0x6b, 0x15});
  auto result = decode(std::move(dg));
  ASSERT_TRUE(result.has_value());
  auto& frame = *result.value();
  EXPECT_EQ(frame.ctrl, SiyiFrameCtrl::Response);
  EXPECT_EQ(frame.seq, 16);
  EXPECT_EQ(static_cast<uint8_t>(frame.cmd), 0x0f);
  ASSERT_EQ(frame.data.size(), 2u);
  EXPECT_EQ(frame.data[0], 0x04);
  EXPECT_EQ(frame.data[1], 0x05);
}

// 55 66 01 00 00 00 00 01 64 c4
// ctrl=Response, data_len=0, seq=0, cmd=RequestGimbalCameraFirmwareVersion
TEST(Decode, ValidFrameFirmwareVersionCmd) {
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x01, 0x64, 0xc4});
  auto result = decode(std::move(dg));
  ASSERT_TRUE(result.has_value());
  auto& frame = *result.value();
  EXPECT_EQ(frame.cmd, SiyiFrameCmd::RequestGimbalCameraFirmwareVersion);
  EXPECT_TRUE(frame.data.empty());
}

// 55 66 01 00 00 00 00 02 07 f4
// ctrl=Response, data_len=0, seq=0, cmd=RequestGimbalCameraHardwareID
TEST(Decode, ValidFrameHardwareIDCmd) {
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x02, 0x07, 0xf4});
  auto result = decode(std::move(dg));
  ASSERT_TRUE(result.has_value());
  auto& frame = *result.value();
  EXPECT_EQ(frame.cmd, SiyiFrameCmd::RequestGimbalCameraHardwareID);
}

// ============================================================
// Decode: ошибки
// ============================================================

TEST(Decode, TooShort_Empty) {
  auto dg = makeDatagram({});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::TooShort);
}

TEST(Decode, TooShort_NineBytes) {
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x19, 0x5D});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::TooShort);
}

TEST(Decode, BadStx_WrongMagic) {
  // Заменяем STX: 0xAA, 0xBB вместо 0x55, 0x66
  auto dg = makeDatagram({0xAA, 0xBB, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x19, 0x5D, 0x57});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::BadStx);
}

TEST(Decode, BadStx_SwappedBytes) {
  // STX в BE порядке вместо LE
  auto dg = makeDatagram({0x66, 0x55, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x19, 0x5D, 0x57});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::BadStx);
}

TEST(Decode, LengthMismatch_TooLong) {
  // Корректный STX, но data_len=5, а реальных байт данных нет
  // buffer.size() = 10, но ожидается 8 + 5 + 2 = 15
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x05, 0x00,
                          0x00, 0x00, 0x19, 0x00, 0x00});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::LengthMismatch);
}

TEST(Decode, LengthMismatch_TooShort) {
  // data_len=0, но буфер содержит лишние байты (11 вместо 10)
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x19, 0x5D, 0x57, 0xFF});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::LengthMismatch);
}

TEST(Decode, CrcMismatch) {
  // Берём валидный фрейм и портим CRC
  // Оригинал: 55 66 01 00 00 00 00 19 5D 57
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x00, 0x00,
                          0x00, 0x00, 0x19, 0x00, 0x00});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::CrcMismatch);
}

TEST(Decode, CrcMismatch_DataCorrupted) {
  // Берём валидный фрейм с данными и меняем один байт данных
  // Оригинал: 55 66 01 01 00 00 00 05 01 8d 64
  auto dg = makeDatagram({0x55, 0x66, 0x01, 0x01, 0x00,
                          0x00, 0x00, 0x05, 0xFF, 0x8d, 0x64});
  auto result = decode(std::move(dg));
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), DecodeFrameError::CrcMismatch);
}

// ============================================================
// Encode: проверка формирования буфера
// ============================================================

TEST(Encode, FrameNoData) {
  // Кодируем фрейм: ctrl=Response, seq=0, cmd=0x19, data={}
  // Ожидаемый результат: 55 66 01 00 00 00 00 19 5D 57
  auto frame = makeFrame(SiyiFrameCtrl::Response, 0,
                         SiyiFrameCmd::RequestGimbalCameraPresentWorkingMode);
  auto dg = encode(std::move(frame));

  const std::vector<uint8_t> expected = {
      0x55, 0x66, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x19, 0x5D, 0x57};
  EXPECT_EQ(dg->buffer, expected);
}

TEST(Encode, FrameFirmwareVersion) {
  // 55 66 01 00 00 00 00 01 64 c4
  auto frame = makeFrame(SiyiFrameCtrl::Response, 0,
                         SiyiFrameCmd::RequestGimbalCameraFirmwareVersion);
  auto dg = encode(std::move(frame));

  const std::vector<uint8_t> expected = {
      0x55, 0x66, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x01, 0x64, 0xc4};
  EXPECT_EQ(dg->buffer, expected);
}

TEST(Encode, FrameHardwareID) {
  // 55 66 01 00 00 00 00 02 07 f4
  auto frame = makeFrame(SiyiFrameCtrl::Response, 0,
                         SiyiFrameCmd::RequestGimbalCameraHardwareID);
  auto dg = encode(std::move(frame));

  const std::vector<uint8_t> expected = {
      0x55, 0x66, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x02, 0x07, 0xf4};
  EXPECT_EQ(dg->buffer, expected);
}

TEST(Encode, FrameWithOneByteData) {
  // Ожидаем: 55 66 01 01 00 00 00 05 01 8d 64
  auto frame = makeFrame(SiyiFrameCtrl::Response, 0,
                         static_cast<SiyiFrameCmd>(0x05), {0x01});
  auto dg = encode(std::move(frame));

  const std::vector<uint8_t> expected = {
      0x55, 0x66, 0x01, 0x01, 0x00,
      0x00, 0x00, 0x05, 0x01, 0x8d, 0x64};
  EXPECT_EQ(dg->buffer, expected);
}

TEST(Encode, FrameWithTwoBytesDataAndNonZeroSeq) {
  // Ожидаем: 55 66 01 02 00 10 00 0f 04 05 6b 15
  auto frame = makeFrame(SiyiFrameCtrl::Response, 16,
                         static_cast<SiyiFrameCmd>(0x0f), {0x04, 0x05});
  auto dg = encode(std::move(frame));

  const std::vector<uint8_t> expected = {
      0x55, 0x66, 0x01, 0x02, 0x00,
      0x10, 0x00, 0x0f, 0x04, 0x05, 0x6b, 0x15};
  EXPECT_EQ(dg->buffer, expected);
}

TEST(Encode, RequestCtrl) {
  // ctrl=Request (0x00), seq=0, cmd=RequestGimbalAttitude, data={}
  auto frame = makeFrame(SiyiFrameCtrl::Request, 0,
                         SiyiFrameCmd::RequestGimbalAttitude);
  auto dg = encode(std::move(frame));

  ASSERT_GE(dg->buffer.size(), MIN_FRAME_SIZE);
  // STX
  EXPECT_EQ(dg->buffer[0], 0x55);
  EXPECT_EQ(dg->buffer[1], 0x66);
  // ctrl = 0
  EXPECT_EQ(dg->buffer[2], 0x00);
  // data_len = 0
  EXPECT_EQ(dg->buffer[3], 0x00);
  EXPECT_EQ(dg->buffer[4], 0x00);
  // seq = 0
  EXPECT_EQ(dg->buffer[5], 0x00);
  EXPECT_EQ(dg->buffer[6], 0x00);
  // cmd = 0x0d
  EXPECT_EQ(dg->buffer[7], 0x0d);
  // Общий размер
  EXPECT_EQ(dg->buffer.size(), 10u);
}

// ============================================================
// Round-trip: encode → decode
// ============================================================

TEST(RoundTrip, NoData) {
  auto original = makeFrame(SiyiFrameCtrl::Response, 42,
                            SiyiFrameCmd::RequestGimbalCameraFirmwareVersion);
  auto dg = encode(std::move(original));
  auto result = decode(std::move(dg));

  ASSERT_TRUE(result.has_value());
  auto& decoded = *result.value();
  EXPECT_EQ(decoded.ctrl, SiyiFrameCtrl::Response);
  EXPECT_EQ(decoded.seq, 42);
  EXPECT_EQ(decoded.cmd, SiyiFrameCmd::RequestGimbalCameraFirmwareVersion);
  EXPECT_TRUE(decoded.data.empty());
}

TEST(RoundTrip, WithData) {
  std::vector<uint8_t> payload = {0x10, 0x20, 0x30, 0x40};
  auto original = makeFrame(SiyiFrameCtrl::Request, 1000,
                            SiyiFrameCmd::SetGimbalRotation, payload);
  auto dg = encode(std::move(original));
  auto result = decode(std::move(dg));

  ASSERT_TRUE(result.has_value());
  auto& decoded = *result.value();
  EXPECT_EQ(decoded.ctrl, SiyiFrameCtrl::Request);
  EXPECT_EQ(decoded.seq, 1000);
  EXPECT_EQ(decoded.cmd, SiyiFrameCmd::SetGimbalRotation);
  EXPECT_EQ(decoded.data, payload);
}

TEST(RoundTrip, MaxSeq) {
  auto original = makeFrame(SiyiFrameCtrl::Response, 0xFFFF,
                            SiyiFrameCmd::SetCenter);
  auto dg = encode(std::move(original));
  auto result = decode(std::move(dg));

  ASSERT_TRUE(result.has_value());
  auto& decoded = *result.value();
  EXPECT_EQ(decoded.seq, 0xFFFF);
  EXPECT_EQ(decoded.cmd, SiyiFrameCmd::SetCenter);
}

TEST(RoundTrip, LargePayload) {
  std::vector<uint8_t> payload(256);
  for (size_t i = 0; i < payload.size(); ++i) {
    payload[i] = static_cast<uint8_t>(i & 0xFF);
  }
  auto original = makeFrame(SiyiFrameCtrl::Request, 7,
                            SiyiFrameCmd::RequestDataStream, payload);
  auto dg = encode(std::move(original));
  auto result = decode(std::move(dg));

  ASSERT_TRUE(result.has_value());
  auto& decoded = *result.value();
  EXPECT_EQ(decoded.ctrl, SiyiFrameCtrl::Request);
  EXPECT_EQ(decoded.seq, 7);
  EXPECT_EQ(decoded.cmd, SiyiFrameCmd::RequestDataStream);
  EXPECT_EQ(decoded.data, payload);
}
