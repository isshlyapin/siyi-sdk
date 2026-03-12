#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include <termios.h>
#include <unistd.h>

#include "siyi-sdk/siyi_client.hxx"

namespace {

constexpr float kAngleStepDeg = 5.0f;
constexpr int kSpeedStep = 5;
constexpr uint16_t kAutoFocusCenterX = 960;
constexpr uint16_t kAutoFocusCenterY = 540;

int clampSpeed(int value) {
  if (value > 100) {
    return 100;
  }
  if (value < -100) {
    return -100;
  }
  return value;
}

float clampAngle(float value) {
  if (value > 180.0f) {
    return 180.0f;
  }
  if (value < -180.0f) {
    return -180.0f;
  }
  return value;
}

class TerminalRawMode {
public:
  TerminalRawMode() {
    if (tcgetattr(STDIN_FILENO, &original_) != 0) {
      std::perror("tcgetattr");
      std::exit(EXIT_FAILURE);
    }

    termios raw = original_;
    raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
      std::perror("tcsetattr");
      std::exit(EXIT_FAILURE);
    }
  }

  ~TerminalRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_);
  }

  TerminalRawMode(const TerminalRawMode&) = delete;
  TerminalRawMode& operator=(const TerminalRawMode&) = delete;

private:
  termios original_{};
};

enum class Key : int {
  None = 0,
  ArrowUp,
  ArrowDown,
  ArrowRight,
  ArrowLeft,
  Char
};

struct KeyEvent {
  Key key{Key::None};
  char value{0};
};

KeyEvent readKeyEvent() {
  unsigned char ch = 0;
  const ssize_t count = read(STDIN_FILENO, &ch, 1);
  if (count <= 0) {
    return {};
  }

  if (ch == 0x1B) {
    unsigned char seq[2] = {0, 0};
    const ssize_t r1 = read(STDIN_FILENO, &seq[0], 1);
    const ssize_t r2 = read(STDIN_FILENO, &seq[1], 1);
    if (r1 == 1 && r2 == 1 && seq[0] == '[') {
      switch (seq[1]) {
      case 'A':
        return {Key::ArrowUp, 0};
      case 'B':
        return {Key::ArrowDown, 0};
      case 'C':
        return {Key::ArrowRight, 0};
      case 'D':
        return {Key::ArrowLeft, 0};
      default:
        break;
      }
    }
    return {};
  }

  return {Key::Char, static_cast<char>(ch)};
}

void printHelp() {
  std::cout
    << "\n=== Ручное управление SIYI ===\n"
    << "Стрелки: точные углы yaw/pitch (шаг 5°)\n"
    << "  ↑/↓ : pitch +/-5°\n"
    << "  ←/→ : yaw   -/+5°\n"
    << "WASD: скорости yaw/pitch (шаг 5, диапазон [-100;100])\n"
    << "  W/S : pitch speed +/-5\n"
    << "  A/D : yaw   speed -/+5\n"
    << "  Space: сброс скоростей в 0\n"
    << "Режим подвеса:\n"
    << "  1: Lock, 2: Follow, 3: FPV\n"
    << "Zoom:\n"
    << "  B: Zoom Out, N: Stop Zoom, M: Zoom In\n"
    << "Focus:\n"
    << "  J: Near, L: Far, K: Stop\n"
    << "Прочее:\n"
    << "  F: автофокус по центру кадра\n"
    << "  P: сделать фото\n"
    << "  C: центрировать подвес\n"
    << "  I: запросить и вывести краткий state\n"
    << "  H: помощь\n"
    << "  Q: выход\n\n";
}

void printStateBrief(siyi::SiyiClient& client) {
  client.requestGimbalAttitude();
  client.requestGimbalCameraPresentWorkingMode();
  client.requestCurrentZoomValue();

  std::this_thread::sleep_for(std::chrono::milliseconds(60));

  const auto angle = client.getGimbalAngleState();
  const auto mode = client.getGimbalModeState();
  const auto zoom = client.getCurrentZoomState();

  std::cout << "[STATE] ";
  if (angle.value) {
    std::cout << "yaw=" << angle.value->yaw
              << " pitch=" << angle.value->pitch
              << " roll=" << angle.value->roll << " | ";
  } else {
    std::cout << "angles=n/a | ";
  }

  if (mode.value) {
    std::cout << "mode=" << static_cast<int>(*mode.value) << " | ";
  } else {
    std::cout << "mode=n/a | ";
  }

  if (zoom.value) {
    std::cout << "zoom=" << *zoom.value;
  } else {
    std::cout << "zoom=n/a";
  }

  std::cout << '\n';
}

void printActionResult(const std::string& action, bool ok) {
  std::cout << '[' << (ok ? "OK" : "ERR") << "] " << action << '\n';
}

} // namespace

int main() {
  using namespace std::chrono_literals;

  siyi::SiyiClient client;

  printHelp();
  printActionResult("requestDataStream(Hz10)", client.requestDataStream(siyi::DataStreamHz::Hz10));

  float targetYaw = 0.0f;
  float targetPitch = 0.0f;
  int yawSpeed = 0;
  int pitchSpeed = 0;

  TerminalRawMode rawMode;

  bool running = true;
  auto lastStatePoll = std::chrono::steady_clock::now();

  while (running) {
    const auto event = readKeyEvent();

    if (event.key == Key::ArrowUp) {
      targetPitch = clampAngle(targetPitch + kAngleStepDeg);
      printActionResult("setControlAngleToGimbal", client.setControlAngleToGimbal(targetYaw, targetPitch));
      std::cout << "  target yaw=" << targetYaw << " pitch=" << targetPitch << '\n';
    } else if (event.key == Key::ArrowDown) {
      targetPitch = clampAngle(targetPitch - kAngleStepDeg);
      printActionResult("setControlAngleToGimbal", client.setControlAngleToGimbal(targetYaw, targetPitch));
      std::cout << "  target yaw=" << targetYaw << " pitch=" << targetPitch << '\n';
    } else if (event.key == Key::ArrowRight) {
      targetYaw = clampAngle(targetYaw + kAngleStepDeg);
      printActionResult("setControlAngleToGimbal", client.setControlAngleToGimbal(targetYaw, targetPitch));
      std::cout << "  target yaw=" << targetYaw << " pitch=" << targetPitch << '\n';
    } else if (event.key == Key::ArrowLeft) {
      targetYaw = clampAngle(targetYaw - kAngleStepDeg);
      printActionResult("setControlAngleToGimbal", client.setControlAngleToGimbal(targetYaw, targetPitch));
      std::cout << "  target yaw=" << targetYaw << " pitch=" << targetPitch << '\n';
    } else if (event.key == Key::Char) {
      switch (event.value) {
      case 'w':
      case 'W':
        pitchSpeed = clampSpeed(pitchSpeed + kSpeedStep);
        printActionResult("setGimbalRotation", client.setGimbalRotation(static_cast<int8_t>(yawSpeed), static_cast<int8_t>(pitchSpeed)));
        std::cout << "  speed yaw=" << yawSpeed << " pitch=" << pitchSpeed << '\n';
        break;
      case 's':
      case 'S':
        pitchSpeed = clampSpeed(pitchSpeed - kSpeedStep);
        printActionResult("setGimbalRotation", client.setGimbalRotation(static_cast<int8_t>(yawSpeed), static_cast<int8_t>(pitchSpeed)));
        std::cout << "  speed yaw=" << yawSpeed << " pitch=" << pitchSpeed << '\n';
        break;
      case 'a':
      case 'A':
        yawSpeed = clampSpeed(yawSpeed - kSpeedStep);
        printActionResult("setGimbalRotation", client.setGimbalRotation(static_cast<int8_t>(yawSpeed), static_cast<int8_t>(pitchSpeed)));
        std::cout << "  speed yaw=" << yawSpeed << " pitch=" << pitchSpeed << '\n';
        break;
      case 'd':
      case 'D':
        yawSpeed = clampSpeed(yawSpeed + kSpeedStep);
        printActionResult("setGimbalRotation", client.setGimbalRotation(static_cast<int8_t>(yawSpeed), static_cast<int8_t>(pitchSpeed)));
        std::cout << "  speed yaw=" << yawSpeed << " pitch=" << pitchSpeed << '\n';
        break;
      case ' ':
        yawSpeed = 0;
        pitchSpeed = 0;
        printActionResult("setGimbalRotation(0,0)", client.setGimbalRotation(0, 0));
        break;
      case '1':
        printActionResult("mode Lock", client.photoAndRecord(siyi::PhotoRecordAction::SwitchLockMode));
        break;
      case '2':
        printActionResult("mode Follow", client.photoAndRecord(siyi::PhotoRecordAction::SwitchFollowMode));
        break;
      case '3':
        printActionResult("mode FPV", client.photoAndRecord(siyi::PhotoRecordAction::SwitchFpvMode));
        break;
      case 'b':
      case 'B':
        printActionResult("zoom out", client.setManualZoomAndAutoFocus(siyi::ZoomDirection::ZoomOut));
        break;
      case 'n':
      case 'N':
        printActionResult("zoom stop", client.setManualZoomAndAutoFocus(siyi::ZoomDirection::Stop));
        break;
      case 'm':
      case 'M':
        printActionResult("zoom in", client.setManualZoomAndAutoFocus(siyi::ZoomDirection::ZoomIn));
        break;
      case 'j':
      case 'J':
        printActionResult("focus near", client.setManualFocus(siyi::FocusDirection::FocusNear));
        break;
      case 'l':
      case 'L':
        printActionResult("focus far", client.setManualFocus(siyi::FocusDirection::FocusFar));
        break;
      case 'k':
      case 'K':
        printActionResult("focus stop", client.setManualFocus(siyi::FocusDirection::Stop));
        break;
      case 'f':
      case 'F':
        printActionResult("autofocus(center)", client.setAutoFocus(kAutoFocusCenterX, kAutoFocusCenterY));
        break;
      case 'p':
      case 'P':
        printActionResult("take photo", client.photoAndRecord(siyi::PhotoRecordAction::TakePhoto));
        break;
      case 'c':
      case 'C':
        targetYaw = 0.0f;
        targetPitch = 0.0f;
        printActionResult("setCenter", client.setCenter());
        break;
      case 'i':
      case 'I':
        printStateBrief(client);
        break;
      case 'h':
      case 'H':
        printHelp();
        break;
      case 'q':
      case 'Q':
        running = false;
        break;
      default:
        break;
      }
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - lastStatePoll >= 2s) {
      client.requestGimbalAttitude();
      lastStatePoll = now;
    }

    std::this_thread::sleep_for(20ms);
  }

  printActionResult("setGimbalRotation(0,0)", client.setGimbalRotation(0, 0));
  printActionResult("zoom stop", client.setManualZoomAndAutoFocus(siyi::ZoomDirection::Stop));
  printActionResult("focus stop", client.setManualFocus(siyi::FocusDirection::Stop));

  std::cout << "Выход.\n";
  return 0;
}
