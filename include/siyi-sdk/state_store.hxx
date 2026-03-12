#pragma once

#include <mutex>
#include <utility>

#include "siyi-sdk/state.hxx"

namespace siyi {

class SiyiStateStore {
public:
  SiyiState snapshot() const {
    std::scoped_lock lock(mutex_);
    return state_;
  }

  FirmwareVersionState firmware() const {
    std::scoped_lock lock(mutex_);
    return state_.firmware;
  }

  HardwareIDState hardware() const {
    std::scoped_lock lock(mutex_);
    return state_.hardware;
  }

  GimbalAngleState gimbalAngle() const {
    std::scoped_lock lock(mutex_);
    return state_.gimbalAngle;
  }

  GimbalRotateSpeedState gimbalRotateSpeed() const {
    std::scoped_lock lock(mutex_);
    return state_.gimbalRotateSpeed;
  }

  GimbalModeState gimbalMode() const {
    std::scoped_lock lock(mutex_);
    return state_.gimbalMode;
  }

  MountingDirectionState mountingDirection() const {
    std::scoped_lock lock(mutex_);
    return state_.mountingDirection;
  }

  GimbalConfigInfoState gimbalConfigInfo() const {
    std::scoped_lock lock(mutex_);
    return state_.gimbalConfigInfo;
  }

  FunctionFeedbackInfoState functionFeedbackInfo() const {
    std::scoped_lock lock(mutex_);
    return state_.functionFeedbackInfo;
  }

  CurrentZoomState currentZoom() const {
    std::scoped_lock lock(mutex_);
    return state_.currentZoom;
  }

  MaxZoomState maxZoom() const {
    std::scoped_lock lock(mutex_);
    return state_.maxZoom;
  }

  GimbalCameraImageModeState imageType() const {
    std::scoped_lock lock(mutex_);
    return state_.imageType;
  }

  RangefinderDataState rangefinder() const {
    std::scoped_lock lock(mutex_);
    return state_.rangefinder;
  }

  ThermalPaletteState thermalPalette() const {
    std::scoped_lock lock(mutex_);
    return state_.thermalPalette;
  }

  ThermalGainState thermalGain() const {
    std::scoped_lock lock(mutex_);
    return state_.thermalGain;
  }

  ThermalPointMeasurementState thermalPointMeasurement() const {
    std::scoped_lock lock(mutex_);
    return state_.thermalPointMeasurement;
  }

  ThermalAreaMeasurementState thermalAreaMeasurement() const {
    std::scoped_lock lock(mutex_);
    return state_.thermalAreaMeasurement;
  }

  ThermalFullFrameMeasurementState thermalFullFrameMeasurement() const {
    std::scoped_lock lock(mutex_);
    return state_.thermalFullFrameMeasurement;
  }

  void updateFirmwareVersions(const FirmwareVersion& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.firmware.value = value;
    touch(state_.firmware.meta, now);
  }

  void updateFirmwareVersion(FirmwareVersion&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.firmware.value = std::move(value);
    touch(state_.firmware.meta, now);
  }

  void updateHardwareID(const HardwareID& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.hardware.value = value;
    touch(state_.hardware.meta, now);
  }

  void updateHardwareID(HardwareID&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.hardware.value = std::move(value);
    touch(state_.hardware.meta, now);
  }

  void updateGimbalAngle(const GimbalAngle& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.gimbalAngle.value = value;
    touch(state_.gimbalAngle.meta, now);
  }  
  
  void updateGimbalAngle(GimbalAngle&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.gimbalAngle.value = std::move(value);
    touch(state_.gimbalAngle.meta, now);
  }
  
  void updateGimbalRotateSpeed(const GimbalRotateSpeed& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.gimbalRotateSpeed.value = value;
    touch(state_.gimbalRotateSpeed.meta, now);
  }

  void updateGimbalRotateSpeed(GimbalRotateSpeed&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.gimbalRotateSpeed.value = std::move(value);
    touch(state_.gimbalRotateSpeed.meta, now);
  }

  void updateGimbalMode(const uint8_t value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.gimbalMode.value = static_cast<GimbalMode>(value);
    touch(state_.gimbalMode.meta, now);
  }

  void updateMountingDirection(const uint8_t value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.mountingDirection.value = static_cast<MountingDirection>(value);
    touch(state_.mountingDirection.meta, now);
  }

  // ---- Camera state updaters ----

  void updateGimbalConfigInfo(const GimbalConfigInfo& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.gimbalConfigInfo.value = value;
    touch(state_.gimbalConfigInfo.meta, now);
  }

  void updateGimbalConfigInfo(GimbalConfigInfo&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.gimbalConfigInfo.value = std::move(value);
    touch(state_.gimbalConfigInfo.meta, now);
  }

  void updateFunctionFeedbackInfo(FunctionFeedbackType value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.functionFeedbackInfo.value = value;
    touch(state_.functionFeedbackInfo.meta, now);
  }

  void updateCurrentZoom(float value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.currentZoom.value = value;
    touch(state_.currentZoom.meta, now);
  }

  void updateMaxZoom(float value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.maxZoom.value = value;
    touch(state_.maxZoom.meta, now);
  }

  void updateImageType(GimbalCameraImageMode value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.imageType.value = value;
    touch(state_.imageType.meta, now);
  }

  // ---- Rangefinder state updater ----

  void updateRangefinder(const RangefinderData& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.rangefinder.value = value;
    touch(state_.rangefinder.meta, now);
  }

  void updateRangefinder(RangefinderData&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.rangefinder.value = std::move(value);
    touch(state_.rangefinder.meta, now);
  }

  // ---- Thermal state updaters ----

  void updateThermalPalette(ThermalPalette value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalPalette.value = value;
    touch(state_.thermalPalette.meta, now);
  }

  void updateThermalGain(ThermalGain value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalGain.value = value;
    touch(state_.thermalGain.meta, now);
  }

  void updateThermalPointMeasurement(const ThermalPointMeasurement& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalPointMeasurement.value = value;
    touch(state_.thermalPointMeasurement.meta, now);
  }

  void updateThermalPointMeasurement(ThermalPointMeasurement&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalPointMeasurement.value = std::move(value);
    touch(state_.thermalPointMeasurement.meta, now);
  }

  void updateThermalAreaMeasurement(const ThermalAreaMeasurement& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalAreaMeasurement.value = value;
    touch(state_.thermalAreaMeasurement.meta, now);
  }

  void updateThermalAreaMeasurement(ThermalAreaMeasurement&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalAreaMeasurement.value = std::move(value);
    touch(state_.thermalAreaMeasurement.meta, now);
  }

  void updateThermalFullFrameMeasurement(const ThermalFullFrameMeasurement& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalFullFrameMeasurement.value = value;
    touch(state_.thermalFullFrameMeasurement.meta, now);
  }

  void updateThermalFullFrameMeasurement(ThermalFullFrameMeasurement&& value, SteadyTimePoint now = nowTs()) {
    std::scoped_lock lock(mutex_);
    state_.thermalFullFrameMeasurement.value = std::move(value);
    touch(state_.thermalFullFrameMeasurement.meta, now);
  }

private:
  static SteadyTimePoint nowTs() {
    return std::chrono::steady_clock::now();
  }

  static void touch(StateMeta& meta, SteadyTimePoint now) {
    meta.initialized = true;
    meta.updatedAt = now;
  }

  mutable std::mutex mutex_;
  SiyiState state_;
};

}  // namespace siyi