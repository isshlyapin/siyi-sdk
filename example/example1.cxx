#include <chrono>
#include <thread>
#include <iostream>

#include "siyi-sdk/siyi_client.hxx"

auto print_firmware_version_state = [](const siyi::FirmwareVersionState& state) {
  if (state.value) {
    std::cout << "Firmware version: " << '\n';
    std::cout << "  Camera: " << static_cast<int>(state.value->camera.major) << "."
                              << static_cast<int>(state.value->camera.minor) << "."
                              << static_cast<int>(state.value->camera.patch) << '\n';
    
    std::cout << "  Gimbal: " << static_cast<int>(state.value->gimbal.major) << "."
                              << static_cast<int>(state.value->gimbal.minor) << "."
                              << static_cast<int>(state.value->gimbal.patch) << '\n';

    std::cout << "  Zoom: " << static_cast<int>(state.value->zoom.major) << "."
                            << static_cast<int>(state.value->zoom.minor) << "."
                            << static_cast<int>(state.value->zoom.patch) << "\n\n";
  } else {
    std::cout << "Firmware version not available\n\n";
  }
};

auto print_hardware_id_state = [](const siyi::HardwareIDState& state) {
  if (state.value) {
    std::cout << "Hardware ID: " << std::setw(2) << std::hex << state.value->id << " (" << state.value->name << ")\n\n";
  } else {
    std::cout << "Hardware ID not available\n\n";
  }
};

auto print_present_working_mode_state = [](const siyi::GimbalModeState& state) {
  if (state.value) {
    std::cout << "Present working mode: ";
    if (*state.value == siyi::GimbalMode::Lock) {
      std::cout << "Lock\n\n";
    } else if (*state.value == siyi::GimbalMode::Follow) {
      std::cout << "Follow\n\n";
    } else if (*state.value == siyi::GimbalMode::Fpv) {
      std::cout << "FPV\n\n";
    } else {
      std::cout << "Unknown (" << static_cast<int>(*state.value) << ")\n\n";
    }
  } else {
    std::cout << "Present working mode not available\n\n";
  }
};

auto print_gimbal_angle_state = [](const siyi::GimbalAngleState& state) {
  if (state.value) {
    std::cout << "Gimbal angle: " << '\n';
    std::cout << "\tYaw: "   << state.value->yaw << '\n';
    std::cout << "\tPitch: " << state.value->pitch << '\n';
    std::cout << "\tRoll: "  << state.value->roll << "\n\n";
  } else {
    std::cout << "Gimbal angle not available\n\n";
  }
};

auto print_gimbal_rotate_speed_state = [](const siyi::GimbalRotateSpeedState& state) {
  if (state.value) {
    std::cout << "Gimbal rotate speed: " << '\n';
    std::cout << "\tYaw speed: "   << state.value->yaw << '\n';
    std::cout << "\tPitch speed: " << state.value->pitch << '\n';
    std::cout << "\tRoll speed: "  << state.value->roll << "\n\n";
  } else {
    std::cout << "Gimbal rotate speed not available\n\n";
  }
};

int main() {
  using namespace std::chrono_literals;
 
  // siyi::SiyiClient client("192.168.222.11", 5005);
  siyi::SiyiClient client;

  // while (!client.requestGimbalCameraFirmwareVersion()) {
  //   std::cout << "Failed to request firmware version, retrying...\n";
  //   std::this_thread::sleep_for(1s);
  // }
  // while (!client.requestDataStream(siyi::DataStreamHz::Hz100)) {
  //   std::cout << "Failed to request data stream, retrying...\n";
  //   std::this_thread::sleep_for(1s);
  // }

  // client.setCenter();

  while (true) {
    client.requestGimbalCameraFirmwareVersion();
    client.requestGimbalCameraHardwareID();
    client.requestGimbalCameraPresentWorkingMode();
    // client.setGimbalRotation(15, 15);
    // std::this_thread::sleep_for(5s); break;


    // client.setControlAngleToGimbal(-90, 20);
    // client.requestGimbalAttitude();
    client.requestDataStream(siyi::DataStreamHz::Hz100);

    auto frw = client.getFirmwareVersionState();
    print_firmware_version_state(frw);

    auto hardwareId = client.getHardwareIDState();
    print_hardware_id_state(hardwareId);

    auto presentWorkingMode = client.getGimbalModeState();
    print_present_working_mode_state(presentWorkingMode);

    auto gimbalAngle = client.getGimbalAngleState();
    print_gimbal_angle_state(gimbalAngle);

    auto gimbalRotateSpeed = client.getGimbalRotateSpeedState();
    print_gimbal_rotate_speed_state(gimbalRotateSpeed);

    std::this_thread::sleep_for(1s);
  }
  return 0;
}