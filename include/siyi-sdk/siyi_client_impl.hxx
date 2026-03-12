#include <thread>
#include <memory>
#include <cstdint>
#include <string_view>

#include "siyi-sdk/protocol.hxx"
#include "siyi-sdk/state_store.hxx"
#include "siyi-sdk/siyi_client.hxx"
#include "siyi-sdk/siyi_sender.hxx"
#include "siyi-sdk/siyi_receiver.hxx"

namespace siyi {
  
class SiyiClient::Impl {
public:
  Impl(std::string_view adress, uint16_t port);

  ~Impl();

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

  Impl(Impl&&) noexcept = delete;
  Impl& operator=(Impl&&) noexcept = delete;

  // Управление
  bool requestGimbalCameraFirmwareVersion();
  bool requestGimbalCameraHardwareID();
  bool requestGimbalCameraPresentWorkingMode();
  bool setAutoFocus(uint16_t touch_x, uint16_t touch_y);
  bool setManualZoomAndAutoFocus(ZoomDirection direction);
  bool setAbsoluteZoomAndAutoFocus(float zoom);
  bool requestMaxZoomValue();
  bool requestCurrentZoomValue();
  bool setManualFocus(FocusDirection direction);
  bool setGimbalRotation(int8_t yawSpeed, int8_t pitchSpeed);
  bool setCenter();
  bool requestGimbalConfigInfo();
  bool photoAndRecord(PhotoRecordAction action);
  bool requestGimbalAttitude();
  bool setControlAngleToGimbal(float yaw, float pitch);
  bool requestGimbalCameraImageMode();
  bool setImageModeToGimbalCamera(GimbalCameraImageMode type);
  bool requestDataStream(DataStreamHz hz);

  // Получение информации о состоянии
  [[nodiscard]] SiyiState getState() const;
  [[nodiscard]] FirmwareVersionState getFirmwareVersionState() const;
  [[nodiscard]] HardwareIDState getHardwareIDState() const;
  [[nodiscard]] GimbalAngleState getGimbalAngleState() const;
  [[nodiscard]] GimbalRotateSpeedState getGimbalRotateSpeedState() const;
  [[nodiscard]] GimbalModeState getGimbalModeState() const;
  [[nodiscard]] MountingDirectionState getMountingDirectionState() const;
  [[nodiscard]] GimbalConfigInfoState getGimbalConfigInfoState() const;
  [[nodiscard]] FunctionFeedbackInfoState getFunctionFeedbackInfoState() const;
  [[nodiscard]] CurrentZoomState getCurrentZoomState() const;
  [[nodiscard]] MaxZoomState getMaxZoomState() const;
  [[nodiscard]] GimbalCameraImageModeState getGimbalCameraImageModeState() const;

private:
  void handleFrame(std::unique_ptr<protocol::SiyiFrame> frame);
  void handleFirmwareVersionResponse(const protocol::SiyiFrame& frame);
  void handleHardwareIDResponse(const protocol::SiyiFrame& frame);
  void handlePresentWorkingModeResponse(const protocol::SiyiFrame& frame);
  void handleManualZoomAndAutoFocusResponse(const protocol::SiyiFrame& frame);
  void handleAbsoluteZoomAndAutoFocusResponse(const protocol::SiyiFrame& frame);
  void handleMaxZoomValueResponse(const protocol::SiyiFrame& frame);
  void handleCurrentZoomValueResponse(const protocol::SiyiFrame& frame);
  void handleGimbalConfigInfoResponse(const protocol::SiyiFrame& frame);
  void handleFunctionFeedbackInfoResponse(const protocol::SiyiFrame& frame);
  void handleGimbalAttitudeResponse(const protocol::SiyiFrame& frame);
  void handleControlAngleToGimbalResponse(const protocol::SiyiFrame& frame);
  void handleGimbalCameraImageModeResponse(const protocol::SiyiFrame& frame);

  void receiverLoop();

  std::shared_ptr<ITransport> transport_;
  SiyiStateStore stateStore_;
  SiyiSender sender_;
  SiyiReceiver receiver_;
  std::jthread receiverThread_;
  std::atomic<bool> running_{false};
};

} // namespace siyi