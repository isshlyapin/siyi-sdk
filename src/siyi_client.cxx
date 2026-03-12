#include <ios>
#include <memory>
#include <cstdint>
#include <string_view>

#include "siyi-sdk/mydef.hxx"
#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/itransport.hxx"
#include "siyi-sdk/siyi_client.hxx"
#include "siyi-sdk/siyi_client_impl.hxx"

namespace {

constexpr auto print_datagram = [](const siyi::DataGram& datagram){
  dbgs << "Datagram: ";
  for (const auto byte : datagram.buffer) {
    dbgs << std::hex << std::setw(2) << std::setfill('0') 
         << static_cast<int>(byte) << " ";
  }
  dbgs << std::dec << "\n\n";
};

} // namespace

namespace siyi {
  
SiyiClient::SiyiClient(std::string_view adress, uint16_t port) 
  : impl_(std::make_unique<Impl>(adress, port)) {}

SiyiClient::~SiyiClient() = default;

SiyiClient::SiyiClient(SiyiClient&&) noexcept = default;
SiyiClient& SiyiClient::operator=(SiyiClient&&) noexcept = default;

bool SiyiClient::requestGimbalCameraFirmwareVersion() {
  return impl_->requestGimbalCameraFirmwareVersion();
}

bool SiyiClient::requestGimbalCameraHardwareID() {
  return impl_->requestGimbalCameraHardwareID();
}

bool SiyiClient::requestGimbalCameraPresentWorkingMode() {
  return impl_->requestGimbalCameraPresentWorkingMode();
}

bool SiyiClient::requestDataStream(DataStreamHz hz) {
  return impl_->requestDataStream(hz);
}

bool SiyiClient::requestGimbalAttitude() {
  return impl_->requestGimbalAttitude();
}

bool SiyiClient::setGimbalRotation(int8_t yawSpeed, int8_t pitchSpeed) {
  return impl_->setGimbalRotation(yawSpeed, pitchSpeed);
}

bool SiyiClient::setCenter() {
  return impl_->setCenter();
}

bool SiyiClient::setControlAngleToGimbal(float yaw, float pitch) {
  return impl_->setControlAngleToGimbal(yaw, pitch);
}

bool SiyiClient::setAutoFocus(uint16_t touch_x, uint16_t touch_y) {
  return impl_->setAutoFocus(touch_x, touch_y);
}

bool SiyiClient::setManualZoomAndAutoFocus(ZoomDirection direction) {
  return impl_->setManualZoomAndAutoFocus(direction);
}

bool SiyiClient::setManualFocus(FocusDirection direction) {
  return impl_->setManualFocus(direction);
}

bool SiyiClient::photoAndRecord(PhotoRecordAction action) {
  return impl_->photoAndRecord(action);
}

bool SiyiClient::requestGimbalConfigInfo() {
  return impl_->requestGimbalConfigInfo();
}

bool SiyiClient::requestMaxZoomValue() {
  return impl_->requestMaxZoomValue();
}

bool SiyiClient::requestCurrentZoomValue() {
  return impl_->requestCurrentZoomValue();
}

bool SiyiClient::setAbsoluteZoomAndAutoFocus(float zoom) {
  return impl_->setAbsoluteZoomAndAutoFocus(zoom);
}

bool SiyiClient::setImageModeToGimbalCamera(GimbalCameraImageMode type) {
  return impl_->setImageModeToGimbalCamera(type);
}

bool SiyiClient::requestGimbalCameraImageMode() {
  return impl_->requestGimbalCameraImageMode();
}

SiyiState SiyiClient::getState() const {
  return impl_->getState();
}

FirmwareVersionState SiyiClient::getFirmwareVersionState() const {
  return impl_->getFirmwareVersionState();
}

HardwareIDState SiyiClient::getHardwareIDState() const {
  return impl_->getHardwareIDState();
}

GimbalAngleState SiyiClient::getGimbalAngleState() const {
  return impl_->getGimbalAngleState();
}

GimbalRotateSpeedState SiyiClient::getGimbalRotateSpeedState() const {
  return impl_->getGimbalRotateSpeedState();
}

GimbalModeState SiyiClient::getGimbalModeState() const {
  return impl_->getGimbalModeState();
}

MountingDirectionState SiyiClient::getMountingDirectionState() const {
  return impl_->getMountingDirectionState();
}

GimbalConfigInfoState SiyiClient::getGimbalConfigInfoState() const {
  return impl_->getGimbalConfigInfoState();
}

FunctionFeedbackInfoState SiyiClient::getFunctionFeedbackInfoState() const {
  return impl_->getFunctionFeedbackInfoState();
}

CurrentZoomState SiyiClient::getCurrentZoomState() const {
  return impl_->getCurrentZoomState();
}

MaxZoomState SiyiClient::getMaxZoomState() const {
  return impl_->getMaxZoomState();
}

GimbalCameraImageModeState SiyiClient::getGimbalCameraImageModeState() const {
  return impl_->getGimbalCameraImageModeState();
}

} // namespace siyi