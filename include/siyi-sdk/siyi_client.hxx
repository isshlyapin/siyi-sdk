/// @file
/// @brief Клиент для взаимодействия с камерами SIYI

#pragma once

#include <memory>
#include <cstdint>
#include <string_view>

#include <siyi-sdk/state.hxx>
#include "siyi-sdk/protocol.hxx"

namespace siyi {

/**
 * @brief Частота отправки данных телеметрии
 */
enum class DataStreamHz : uint8_t {
  HzOFF = 0, ///< Отключить отправку
  Hz2   = 1, ///< 2 Гц
  Hz4   = 2, ///< 4 Гц
  Hz5   = 3, ///< 5 Гц
  Hz10  = 4, ///< 10 Гц
  Hz20  = 5, ///< 20 Гц
  Hz50  = 6, ///< 50 Гц
  Hz100 = 7  ///< 100 Гц
};

/**
 * @brief Направление зума
 */
enum class ZoomDirection : int8_t {
  ZoomOut = -1, ///< Уменьшить зум
  Stop    = 0,  ///< Остановить
  ZoomIn  = 1   ///< Увеличить зум
};

/**
 * @brief Направление фокусировки
 */
enum class FocusDirection : int8_t {
  FocusNear = -1, ///< Ближе
  Stop      = 0,  ///< Остановить
  FocusFar  = 1   ///< Дальше
};

/**
 * @brief Действия с фото и видео
 */
enum class PhotoRecordAction : uint8_t {
  TakePhoto           = 0, ///< Сделать фото
  ToggleRecord        = 2, ///< Начать/остановить запись видео
  SwitchLockMode      = 3, ///< Переключить в режим Lock
  SwitchFollowMode    = 4, ///< Переключить в режим Follow
  SwitchFpvMode       = 5, ///< Переключить в режим FPV
  SetVideoHdmi        = 6, ///< Вывод видео через HDMI
  SetVideoCvbs        = 7, ///< Вывод видео через CVBS
  TurnOffVideoOutput  = 8  ///< Отключить видеовыход
};

/**
 * @brief Основной класс для управления камерой SIYI
 *
 * Предоставляет методы для отправки команд камере и получения её состояния.
 */
class SiyiClient {
public:
  /**
   * @brief Конструктор с указанием адреса и порта (по умолчанию 192.168.144.25:37260)
   * @param address IP-адрес камеры
   * @param port Порт для подключения
   */
  explicit SiyiClient(
    std::string_view adress = protocol::DEFAULT_IP_ADDRESS, 
    uint16_t port = protocol::DEFAULT_PORT
  );

  /**
   * @brief Деструктор.
   */
  ~SiyiClient();

  SiyiClient(const SiyiClient&) = delete;
  SiyiClient& operator=(const SiyiClient&) = delete;

  SiyiClient(SiyiClient&&) noexcept;
  SiyiClient& operator=(SiyiClient&&) noexcept;

  /**
   * @brief Запросить версию прошивки 
   * @return true, если команда успешно отправлена
   */
  bool requestGimbalCameraFirmwareVersion();

  /**
   * @brief Запросить аппаратный ID 
   * @return true, если команда успешно отправлена
   */
  bool requestGimbalCameraHardwareID();

  /**
   * @brief Запросить текущий рабочий режим подвеса
   * @return true, если команда успешно отправлена
   */
  bool requestGimbalCameraPresentWorkingMode();

  /**
   * @brief Установить автофокус по координатам
   * @param touch_x Координата X (в диапазоне длины разрешения)
   * @param touch_y Координата Y (в диапазоне высоты разрешения)
   * @return true, если команда успешно отправлена
   */
  bool setAutoFocus(uint16_t touch_x, uint16_t touch_y);

  /**
   * @brief Установить направление изменения zoom и автофокус
   * @param direction Направление изменения зума
   * @return true, если команда успешно отправлена
   */
  bool setManualZoomAndAutoFocus(ZoomDirection direction);

  /**
   * @brief Установить абсолютное значение зума и автофокус
   * @param zoom Значение зума
   * @return true, если команда успешно отправлена
   */
  bool setAbsoluteZoomAndAutoFocus(float zoom);

  /**
   * @brief Запросить максимальное значение зума
   * @return true, если команда успешно отправлена
   */
  bool requestMaxZoomValue();

  /**
   * @brief Запросить текущее значение зума
   * @return true, если команда успешно отправлена
   */
  bool requestCurrentZoomValue();

  /**
   * @brief Установить направление изменения фокусировки
   * @param direction Направление изменения фокусировки
   * @return true, если команда успешно отправлена
   */
  bool setManualFocus(FocusDirection direction);

  /**
   * @brief Установить скорость вращения подвеса
   * @param yawSpeed Скорость рыскания (-100 ~ 100)
   * @param pitchSpeed Скорость тангажа (-100 ~ 100)
   * @return true, если команда успешно отправлена
   */
  bool setGimbalRotation(int8_t yawSpeed, int8_t pitchSpeed);

  /**
   * @brief Центрировать подвес
   * @return true, если команда успешно отправлена
   */
  bool setCenter();

  /**
   * @brief Запросить информацию о конфигурации подвеса
   * @return true, если команда успешно отправлена
   */
  bool requestGimbalConfigInfo();

  /**
   * @brief Выполнить действие с фото или видео
   * @param action Действие
   * @return true, если команда успешно отправлена
   */
  bool photoAndRecord(PhotoRecordAction action); // TODO(shlyapin): Разобраться с этой командой

  /**
   * @brief Запросить данные о подвесе (текущие углы, скорость вращения)
   * @return true, если команда успешно отправлена
   */
  bool requestGimbalAttitude();

  /**
   * @brief Установить углы подвеса (точность до 0.1 градуса)
   * @param yaw Угол рыскания
   * @param pitch Угол тангажа
   * @return true, если команда успешно отправлена
   */
  bool setControlAngleToGimbal(float yaw, float pitch);

  /**
   * @brief Запросить режим изображения камеры
   * @return true, если команда успешно отправлена
   */
  bool requestGimbalCameraImageMode();

  /**
   * @brief Установить режим изображения камеры
   * @param type Тип режима изображения
   * @return true, если команда успешно отправлена
   */
  bool setImageModeToGimbalCamera(GimbalCameraImageMode type);

  /**
   * @brief Запросить поток данных с определенной частотой
   * @param hz Частота
   * @return true, если команда успешно отправлена
   */
  bool requestDataStream(DataStreamHz hz);

  /**
   * @brief Получить полное состояние камеры
   * @return Структура SiyiState.
   */
  [[nodiscard]] SiyiState getState() const;

  /**
   * @brief Получить состояние версии прошивки
   * @return Структура FirmwareVersionState
   */
  [[nodiscard]] FirmwareVersionState getFirmwareVersionState() const;

  /**
   * @brief Получить состояние аппаратного ID
   * @return Структура HardwareIDState
   */
  [[nodiscard]] HardwareIDState getHardwareIDState() const;

  /**
   * @brief Получить состояние углов подвеса
   * @return Структура GimbalAngleState
   */
  [[nodiscard]] GimbalAngleState getGimbalAngleState() const;

  /**
   * @brief Получить состояние скорости вращения подвеса
   * @return Структура GimbalRotateSpeedState
   */
  [[nodiscard]] GimbalRotateSpeedState getGimbalRotateSpeedState() const;

  /**
   * @brief Получить состояние режима работы подвеса
   * @return Структура GimbalModeState
   */
  [[nodiscard]] GimbalModeState getGimbalModeState() const;

  /**
   * @brief Получить состояние об ориентации подвеса
   * @return Структура MountingDirectionState
   */
  [[nodiscard]] MountingDirectionState getMountingDirectionState() const;

  /**
   * @brief Получить состояние информации о конфигурации подвеса
   * @return Структура GimbalConfigInfoState
   */
  [[nodiscard]] GimbalConfigInfoState getGimbalConfigInfoState() const;

  /**
   * @brief Получить состояние обратной связи о функции
   * @return Структура FunctionFeedbackInfoState
   */
  [[nodiscard]] FunctionFeedbackInfoState getFunctionFeedbackInfoState() const;

  /**
   * @brief Получить текущее значение zoom
   * @return Структура CurrentZoomState
   */
  [[nodiscard]] CurrentZoomState getCurrentZoomState() const;

  /**
   * @brief Получить максимальное значение zoom
   * @return Структура MaxZoomState
   */
  [[nodiscard]] MaxZoomState getMaxZoomState() const;

  /**
   * @brief Получить состояние режима изображения камеры
   * @return Структура GimbalCameraImageModeState
   */
  [[nodiscard]] GimbalCameraImageModeState getGimbalCameraImageModeState() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace siyi