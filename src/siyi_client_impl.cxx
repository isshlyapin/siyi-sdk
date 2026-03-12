#include <thread>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include "siyi-sdk/mydef.hxx"
#include "siyi-sdk/helpers.hxx"
#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/siyi_sender.hxx"
#include "siyi-sdk/state_store.hxx"
#include "siyi-sdk/udp_transport.hxx"
#include "siyi-sdk/siyi_receiver.hxx"
#include "siyi-sdk/siyi_client_impl.hxx"

namespace siyi {

SiyiClient::Impl::Impl(std::string_view adress, uint16_t port) 
  : transport_(std::make_shared<UdpTransport>(adress, port)),
    stateStore_(), sender_(transport_), receiver_(transport_) {
  if (transport_->open() != TransportOpenError::Ok) {
    // TODO(shlyapin): Handle error
    throw std::runtime_error("Failed to open transport");
  }

  running_ = true;
  receiverThread_ = std::jthread(&Impl::receiverLoop, this);
}

SiyiClient::Impl::~Impl() {
  running_ = false;
}

// Управление
bool SiyiClient::Impl::requestGimbalCameraFirmwareVersion() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::RequestGimbalCameraFirmwareVersion,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestGimbalCameraHardwareID() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::RequestGimbalCameraHardwareID,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestGimbalCameraPresentWorkingMode() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::RequestGimbalCameraPresentWorkingMode,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestDataStream(DataStreamHz hz) {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::RequestDataStream,
    .data = {static_cast<uint8_t>(hz)},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestGimbalAttitude() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::RequestGimbalAttitude,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setGimbalRotation(int8_t yawSpeed, int8_t pitchSpeed) {
    protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::SetGimbalRotation,
    .data = {static_cast<uint8_t>(yawSpeed), static_cast<uint8_t>(pitchSpeed)},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setCenter() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::SetCenter,
    .data = {1}, 
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setControlAngleToGimbal(float yaw, float pitch) {
  std::vector<uint8_t> data(4);
  utility::write_i16(data, 0, static_cast<int16_t>(yaw * 10));
  utility::write_i16(data, 2, static_cast<int16_t>(pitch * 10));
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0, // Will be set by sender
    .cmd = protocol::SiyiFrameCmd::SetControlAngleToGimbal,
    .data = std::move(data),
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setAutoFocus(uint16_t touch_x, uint16_t touch_y) {
  std::vector<uint8_t> data(5);
  data[0] = 1; // Start auto focus for once
  utility::write_u16(data, 1, touch_x);
  utility::write_u16(data, 3, touch_y);
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::SetAutoFocus,
    .data = std::move(data),
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setManualZoomAndAutoFocus(ZoomDirection direction) {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::SetManualZoomAndAutoFocus,
    .data = {static_cast<uint8_t>(direction)},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setManualFocus(FocusDirection direction) {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::SetManualFocus,
    .data = {static_cast<uint8_t>(direction)},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::photoAndRecord(PhotoRecordAction action) {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::SetPhotoAndRecord,
    .data = {static_cast<uint8_t>(action)},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestGimbalConfigInfo() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::RequestGimbalConfigInfo,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestMaxZoomValue() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::RequestTheMaxZoomValueInPresent,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestCurrentZoomValue() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::RequestTheZoomValueInPresent,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setAbsoluteZoomAndAutoFocus(float zoom) {
  const auto intPart   = static_cast<uint8_t>(zoom);
  const auto floatPart = static_cast<uint8_t>((zoom - intPart) * 10);
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::SetAbsoluteZoomAndAutoFocus,
    .data = {intPart, floatPart},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::setImageModeToGimbalCamera(GimbalCameraImageMode type) {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::SetImageModeToGimbalCamera,
    .data = {static_cast<uint8_t>(type)},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

bool SiyiClient::Impl::requestGimbalCameraImageMode() {
  protocol::SiyiFrame frame{
    .ctrl = protocol::SiyiFrameCtrl::Response,
    .seq = 0,
    .cmd = protocol::SiyiFrameCmd::RequestGimbalCameraImageMode,
    .data = {},
  };
  return sender_.send(std::make_unique<protocol::SiyiFrame>(std::move(frame)));
}

// Получение информации о состоянии
SiyiState SiyiClient::Impl::getState() const {
  return stateStore_.snapshot();
}

FirmwareVersionState SiyiClient::Impl::getFirmwareVersionState() const {
  return stateStore_.firmware();
}

HardwareIDState SiyiClient::Impl::getHardwareIDState() const {
  return stateStore_.hardware();
}

GimbalAngleState SiyiClient::Impl::getGimbalAngleState() const {
  return stateStore_.gimbalAngle();
}

GimbalRotateSpeedState SiyiClient::Impl::getGimbalRotateSpeedState() const {
  return stateStore_.gimbalRotateSpeed();
}

GimbalModeState SiyiClient::Impl::getGimbalModeState() const {
  return stateStore_.gimbalMode();
}

MountingDirectionState SiyiClient::Impl::getMountingDirectionState() const {
  return stateStore_.mountingDirection();
}

GimbalConfigInfoState SiyiClient::Impl::getGimbalConfigInfoState() const {
  return stateStore_.gimbalConfigInfo();
}

FunctionFeedbackInfoState SiyiClient::Impl::getFunctionFeedbackInfoState() const {
  return stateStore_.functionFeedbackInfo();
}

CurrentZoomState SiyiClient::Impl::getCurrentZoomState() const {
  return stateStore_.currentZoom();
}

MaxZoomState SiyiClient::Impl::getMaxZoomState() const {
  return stateStore_.maxZoom();
}

GimbalCameraImageModeState SiyiClient::Impl::getGimbalCameraImageModeState() const {
  return stateStore_.imageType();
}

void SiyiClient::Impl::handleFrame(std::unique_ptr<protocol::SiyiFrame> frame) {
  switch (frame->cmd) {
    case protocol::SiyiFrameCmd::RequestGimbalCameraFirmwareVersion: {
      dbgs << "Received RequestGimbalCameraFirmwareVersion response\n";
      handleFirmwareVersionResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::RequestGimbalCameraHardwareID: {
      dbgs << "Received RequestGimbalCameraHardwareID response\n";
      handleHardwareIDResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::RequestGimbalCameraPresentWorkingMode: {
      dbgs << "Received RequestGimbalCameraPresentWorkingMode response\n";
      handlePresentWorkingModeResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::SetAutoFocus: {
      dbgs << "Received SetAutoFocus response\n";
      break;
    }
    case protocol::SiyiFrameCmd::SetManualZoomAndAutoFocus: {
      dbgs << "Received SetManualZoom response\n";
      handleManualZoomAndAutoFocusResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::SetManualFocus: {
      dbgs << "Received SetManualFocus response\n";
      break;
    }
    case protocol::SiyiFrameCmd::SetGimbalRotation: {
      dbgs << "Received SetGimbalRotation response\n";
      break;
    }
    case protocol::SiyiFrameCmd::SetCenter: {
      dbgs << "Received SetCenter response\n";
      break;
    }
    case protocol::SiyiFrameCmd::RequestGimbalConfigInfo: {
      dbgs << "Received RequestGimbalConfigInfo response\n";
      handleGimbalConfigInfoResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::RequestFunctionFeedbackInfo: {
      dbgs << "Received FunctionFeedbackInfo\n";
      handleFunctionFeedbackInfoResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::SetPhotoAndRecord: {
      dbgs << "Received SetPhotoAndRecord response\n";
      break;
    }
    case protocol::SiyiFrameCmd::RequestGimbalAttitude: {
      dbgs << "Received RequestGimbalAttitude response\n";
      handleGimbalAttitudeResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::SetControlAngleToGimbal: {
      dbgs << "Received SetControlAngleToGimbal response\n";
      handleControlAngleToGimbalResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::SetAbsoluteZoomAndAutoFocus: {
      dbgs << "Received SetAbsoluteZoom response\n";
      handleAbsoluteZoomAndAutoFocusResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::RequestGimbalCameraImageMode: {
      dbgs << "Received RequestImageType response\n";
      handleGimbalCameraImageModeResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::SetImageModeToGimbalCamera: {
      dbgs << "Received SetImageType response\n";
      handleGimbalCameraImageModeResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::RequestTheMaxZoomValueInPresent: {
      dbgs << "Received RequestMaxZoomValue response\n";
      handleMaxZoomValueResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::RequestTheZoomValueInPresent: {
      dbgs << "Received RequestCurrentZoomValue response\n";
      handleCurrentZoomValueResponse(*frame);
      break;
    }
    case protocol::SiyiFrameCmd::RequestDataStream: {
      dbgs << "Received RequestDataStream response\n";
      break;
    }
    default: {
      dbgs << "Received unknown command: " << static_cast<int>(frame->cmd) << "\n";
    }
  }
}

void SiyiClient::Impl::handleFirmwareVersionResponse(const protocol::SiyiFrame& frame) {
  if (frame.data.size() != 12) {
    dbgs << "Invalid firmware version response size: " << frame.data.size() << "\n";
    return;
  }
  FirmwareVersion version{
    .camera = {.major = frame.data[1], .minor = frame.data[2],  .patch = frame.data[3]},
    .gimbal = {.major = frame.data[5], .minor = frame.data[6],  .patch = frame.data[7]},
    .zoom   = {.major = frame.data[9], .minor = frame.data[10], .patch = frame.data[11]}
  };
  stateStore_.updateFirmwareVersion(std::move(version));
}

void SiyiClient::Impl::handleHardwareIDResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kHardwareIdPayloadSize = 12;
  constexpr const size_t kModelIdSize = 2;
  
  if (frame.data.size() != kHardwareIdPayloadSize) {
    dbgs << "Invalid hardware ID response size: " << frame.data.size() << "\n";
    return;
  }

  // TODO(shlyapin): id тоже приходит в ascii
  HardwareID hardware{
    .id = utility::read_u16(frame.data, 0),
    .name = std::string(frame.data.begin() + kModelIdSize, frame.data.end())
  };
  stateStore_.updateHardwareID(std::move(hardware));
}

void SiyiClient::Impl::handlePresentWorkingModeResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 1;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid present working mode response size: " << frame.data.size() << "\n";
    return;
  }
  stateStore_.updateGimbalMode(frame.data[0]);
}

void SiyiClient::Impl::handleControlAngleToGimbalResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 6;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid control angle to gimbal response size: " << frame.data.size() << "\n";
    return;
  }

  const float yaw   = utility::read_i16_le(frame.data, 0) / 10.0f;
  const float pitch = utility::read_i16_le(frame.data, 2) / 10.0f;
  const float roll  = utility::read_i16_le(frame.data, 4) / 10.0f;
  GimbalAngle angle{
    .yaw   = yaw,
    .pitch = pitch,
    .roll  = roll
  };
  stateStore_.updateGimbalAngle(std::move(angle));
}

void SiyiClient::Impl::handleGimbalAttitudeResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 12;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid gimbal attitude response size: " << frame.data.size() << "\n";
    return;
  }

  const float yaw   = utility::read_i16_le(frame.data, 0) / 10.0f;
  const float pitch = utility::read_i16_le(frame.data, 2) / 10.0f;
  const float roll  = utility::read_i16_le(frame.data, 4) / 10.0f;
  const int16_t yawSpeed   = utility::read_i16_le(frame.data, 6) / 10;
  const int16_t pitchSpeed = utility::read_i16_le(frame.data, 8) / 10;
  const int16_t rollSpeed  = utility::read_i16_le(frame.data, 10) / 10;

  GimbalAngle angle{
    .yaw   = yaw,
    .pitch = pitch,
    .roll  = roll
  }; 
  GimbalRotateSpeed rotateSpeed{
    .yaw   = yawSpeed,
    .pitch = pitchSpeed,
    .roll  = rollSpeed
  };

  stateStore_.updateGimbalAngle(std::move(angle));
  stateStore_.updateGimbalRotateSpeed(std::move(rotateSpeed));
}

void SiyiClient::Impl::handleManualZoomAndAutoFocusResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 2;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid manual zoom response size: " << frame.data.size() << "\n";
    return;
  }

  const float currentZoom = utility::read_u16_le(frame.data, 0) / 10.0f;
  stateStore_.updateCurrentZoom(currentZoom);
}

void SiyiClient::Impl::handleGimbalConfigInfoResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 7;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid gimbal config info response size: " << frame.data.size() << "\n";
    return;
  }

  // byte 0: reserved
  // byte 1: HDR status (0=off, 1=on)
  // byte 2: reserved
  // byte 3: record_sta (0=off, 1=on, 2="TF card slot is empty", 3="Data loss")
  // byte 4: gimbal motion mode
  // byte 5: gimbal mounting direction
  GimbalConfigInfo configInfo{
    .hdrEnabled = frame.data[1] == 1,
    .recordingStatus = static_cast<RecordingStatus>(frame.data[3]),
  };
  stateStore_.updateGimbalConfigInfo(std::move(configInfo));
  stateStore_.updateGimbalMode(frame.data[4]);
  stateStore_.updateMountingDirection(frame.data[5]);
}

void SiyiClient::Impl::handleFunctionFeedbackInfoResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 1;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid function feedback info response size: " << frame.data.size() << "\n";
    return;
  }

  stateStore_.updateFunctionFeedbackInfo(static_cast<FunctionFeedbackType>(frame.data[0]));
}

void SiyiClient::Impl::handleMaxZoomValueResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 2;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid max zoom value response size: " << frame.data.size() << "\n";
    return;
  }

  const float maxZoom = static_cast<float>(frame.data[0]) + (static_cast<float>(frame.data[1]) / 10.0f);
  stateStore_.updateMaxZoom(maxZoom);
}

void SiyiClient::Impl::handleCurrentZoomValueResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 2;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid current zoom value response size: " << frame.data.size() << "\n";
    return;
  }

  const float currentZoom = static_cast<float>(frame.data[0]) + (static_cast<float>(frame.data[1]) / 10.0f);
  stateStore_.updateCurrentZoom(currentZoom);
}

void SiyiClient::Impl::handleAbsoluteZoomAndAutoFocusResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 1;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid absolute zoom response size: " << frame.data.size() << "\n";
    return;
  }
  // ACK: 1 = success. No state update needed.
}

void SiyiClient::Impl::handleGimbalCameraImageModeResponse(const protocol::SiyiFrame& frame) {
  constexpr const size_t kExpectedDataSize = 1;
  if (frame.data.size() != kExpectedDataSize) {
    dbgs << "Invalid image type response size: " << frame.data.size() << "\n";
    return;
  }

  stateStore_.updateImageType(static_cast<GimbalCameraImageMode>(frame.data[0]));
}

void SiyiClient::Impl::receiverLoop() {
  using namespace std::chrono_literals;
  while (running_) {
    auto frame = receiver_.receive();
    if (frame) {
      handleFrame(std::move(frame.value()));
      // TODO(shlyapin): Handle receive error
    }
    // TODO(shlyapin): Delete magic sleep
    std::this_thread::sleep_for(10ms);
  }
}

} // namespace siyi