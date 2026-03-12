#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <expected>

#include "siyi-sdk/itransport.hxx"

namespace siyi::protocol {

constexpr const size_t MIN_FRAME_SIZE = 10; // stx(2) + ctrl(1) + data_len(2) + seq(2) + cmd(1) + crc(2)

constexpr static const uint16_t STX = 0x6655;

constexpr const uint16_t DEFAULT_PORT = 37260;
constexpr const std::string_view DEFAULT_IP_ADDRESS = "192.168.144.25";

enum class DecodeFrameError : uint8_t {
  NoFrame,
  OldFrame,
  TooShort,
  BadStx,
  LengthMismatch,
  CrcMismatch,
};

enum class SiyiFrameCtrl : uint8_t {
  Request  = 0,
  Response = 1,
};

enum class SiyiFrameCmd : uint8_t {
  RequestGimbalCameraFirmwareVersion = 0x01,
  RequestGimbalCameraHardwareID = 0x02,
  RequestGimbalCameraPresentWorkingMode = 0x19,
  SetAutoFocus = 0x04,
  SetManualZoomAndAutoFocus = 0x05,
  SetAbsoluteZoomAndAutoFocus = 0x0F,
  RequestTheMaxZoomValueInPresent = 0x16,
  RequestTheZoomValueInPresent = 0x18,
  SetManualFocus = 0x06,
  SetGimbalRotation = 0x07,
  SetCenter = 0x08,
  RequestGimbalConfigInfo = 0x0A,
  RequestFunctionFeedbackInfo = 0x0B,
  SetPhotoAndRecord = 0x0C,
  RequestGimbalAttitude = 0x0D,
  SetControlAngleToGimbal = 0x0E,
  RequestGimbalCameraCodecSpecs = 0x20, // TODO(shlyapin): Реализовать эту команду
  setCodecSpecsToGimbalCamera = 0x21, // TODO(shlyapin): Реализовать эту команду
  RequestGimbalCameraImageMode = 0x10, 
  SetImageModeToGimbalCamera = 0x11,
  RequestDataStream = 0x25
};

struct SiyiFrame {
  SiyiFrameCtrl ctrl{};
  uint16_t seq{};
  SiyiFrameCmd cmd{};
  std::vector<uint8_t> data;
};

std::expected<std::unique_ptr<SiyiFrame>, DecodeFrameError> 
  decode(std::unique_ptr<DataGram> datagram);

std::unique_ptr<DataGram> 
  encode(std::unique_ptr<SiyiFrame> frame);

} // namespace siyi::protocol