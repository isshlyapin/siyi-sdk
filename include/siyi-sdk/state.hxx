/// @file
/// @brief Определения структур состояния и перечислений для камер SIYI

#pragma once

#include <string>
#include <chrono>
#include <cstdint>
#include <optional>

namespace siyi {

/// @brief Псевдоним для точки времени steady_clock
using SteadyTimePoint = std::chrono::steady_clock::time_point;

/**
 * @brief Метаинформация о состоянии
 */
struct StateMeta {
  bool initialized{false}; ///< Инициализировано ли состояние
  SteadyTimePoint updatedAt; ///< Время последнего обновления
};

// TODO(shlyapin) Тесты на правильные числа
/**
 * @brief Режим работы подвеса
 */
enum class GimbalMode : uint8_t {
  Lock   = 0, ///< Режим блокировки
  Follow = 1, ///< Режим следования
  Fpv    = 2  ///< Режим от первого лица
};

/**
 * @brief Ориентация подвеса
 */
enum class MountingDirection : uint8_t {
  Normal     = 0x01, ///< Обычное
  UpsideDown = 0x02  ///< Вверх ногами
};

/**
 * @brief Статус записи видео
 */
enum class RecordingStatus : uint8_t {
  Off      = 0, ///< Запись выключена
  On       = 1, ///< Запись включена
  NoSdCard = 2, ///< SD-карта отсутствует
  DataLoss = 3  ///< Потеря данных
};

// TODO(shlyapin): Разобраться как работает эта команда
/**
 * @brief Тип обратной связи о функции
 */
enum class FunctionFeedbackType : uint8_t {
  PhotoSuccess   = 0, ///< Фото сделано успешно
  PhotoFail      = 1, ///< Ошибка при фото
  HdrOn          = 2, ///< HDR включен
  HdrOff         = 3, ///< HDR выключен
  RecordFail     = 4  ///< Ошибка записи
};

/**
 * @brief Режим изображения камеры
 */
enum class GimbalCameraImageMode : uint8_t {
  SplitMainZoomAndThermalSubWideAngle = 0, ///< Разделенный: основное - зум, дополнительное - тепловизор (широкоугольный)
  SplitMainWideAngleAndThermalSubZoom = 1, ///< Разделенный: основное - широкоугольный, дополнительное - тепловизор (зум)
  SplitMainZoomAndWideAngleSubThermal = 2, ///< Разделенный: основное - зум, дополнительное - широкоугольный (тепловизор)
  SingleMainZoomSubThermal            = 3, ///< Один: основное - зум, дополнительное - тепловизор
  SingleMainZoomSubWideAngle          = 4, ///< Один: основное - зум, дополнительное - широкоугольный
  SingleMainWideAngleSubThermal       = 5, ///< Один: основное - широкоугольный, дополнительное - тепловизор
  SingleMainWideAngleSubZoom          = 6, ///< Один: основное - широкоугольный, дополнительное - зум
  SingleMainThermalSubZoom            = 7, ///< Один: основное - тепловизор, дополнительное - зум
  SingleMainThermalSubWide            = 8  ///< Один: основное - тепловизор, дополнительное - широкоугольный
};

/**
 * @brief Тепловая палитра
 */
enum class ThermalPalette : uint8_t {
  WhiteHot  = 0,  ///< Белый горячий
  Reserved  = 1,  ///< Зарезервировано
  Sepia     = 2,  ///< Сепия
  Ironbow   = 3,  ///< Железная дуга
  Rainbow   = 4,  ///< Радуга
  Night     = 5,  ///< Ночь
  Aurora    = 6,  ///< Аврора
  RedHot    = 7,  ///< Красный горячий
  Jungle    = 8,  ///< Джунгли
  Medical   = 9,  ///< Медицинский
  BlackHot  = 10, ///< Черный горячий
  GloryHot  = 11  ///< Славный горячий
};

/**
 * @brief Усиление тепловизора
 */
enum class ThermalGain : uint8_t {
  Low  = 0, ///< Низкое
  High = 1  ///< Высокое
};

/**
 * @brief Версия компонента.
 */
struct Version {
  uint8_t major = 0; ///< Мажорная версия
  uint8_t minor = 0; ///< Минорная версия
  uint8_t patch = 0; ///< Патч
};

/**
 * @brief Версии прошивок
 */
struct FirmwareVersion {
  Version camera; ///< Версия камеры
  Version gimbal; ///< Версия подвеса
  Version zoom;   ///< Версия модуля зума
};

/**
 * @brief Состояние версии прошивки
 */
struct FirmwareVersionState {
  StateMeta meta; ///< Метаинформация
  std::optional<FirmwareVersion> value; ///< Значение
};

/**
 * @brief Аппаратный ID
 */
struct HardwareID {
  uint16_t id;   ///< ID
  std::string name; ///< Имя
};

/**
 * @brief Состояние аппаратного ID
 */
struct HardwareIDState {
  StateMeta meta; ///< Метаинформация
  std::optional<HardwareID> value; ///< Значение
};

/**
 * @brief Углы подвеса
 */
struct GimbalAngle {
  float yaw;   ///< Угол рыскания
  float pitch; ///< Угол тангажа
  float roll;  ///< Угол крена
};

/**
 * @brief Состояние углов подвеса
 */
struct GimbalAngleState {
  StateMeta meta; ///< Метаинформация
  std::optional<GimbalAngle> value; ///< Значение
};

/**
 * @brief Скорость вращения подвеса
 */
struct GimbalRotateSpeed {
  int16_t yaw;   ///< Скорость по рысканию
  int16_t pitch; ///< Скорость по тангажу
  int16_t roll;  ///< Скорость по крену
};

/**
 * @brief Состояние скорости вращения подвеса
 */
struct GimbalRotateSpeedState {
  StateMeta meta; ///< Метаинформация
  std::optional<GimbalRotateSpeed> value; ///< Значение
};

/**
 * @brief Состояние режима подвеса
 */
struct GimbalModeState {
  StateMeta meta; ///< Метаинформация
  std::optional<GimbalMode> value; ///< Значение
};

/**
 * @brief Состояние направления монтажа
 */
struct MountingDirectionState {
  StateMeta meta; ///< Метаинформация
  std::optional<MountingDirection> value; ///< Значение
};

/**
 * @brief Информация о конфигурации подвеса
 */
struct GimbalConfigInfo {
  bool hdrEnabled; ///< Включен ли HDR
  RecordingStatus recordingStatus; ///< Статус записи
};

/**
 * @brief Состояние информации о конфигурации подвеса
 */
struct GimbalConfigInfoState {
  StateMeta meta; ///< Метаинформация
  std::optional<GimbalConfigInfo> value; ///< Значение
};

/**
 * @brief Состояние обратной связи о функции
 */
struct FunctionFeedbackInfoState {
  StateMeta meta; ///< Метаинформация
  std::optional<FunctionFeedbackType> value; ///< Значение
};

/**
 * @brief Состояние текущего значения зума
 */
struct CurrentZoomState {
  StateMeta meta; ///< Метаинформация
  std::optional<float> value; ///< Значение
};

/**
 * @brief Состояние максимального значения зума
 */
struct MaxZoomState {
  StateMeta meta; ///< Метаинформация
  std::optional<float> value; ///< Значение
};

/**
 * @brief Состояние режима изображения камеры
 */
struct GimbalCameraImageModeState {
  StateMeta meta; ///< Метаинформация
  std::optional<GimbalCameraImageMode> value; ///< Значение
};

/**
 * @brief Данные с дальномера
 */
struct RangefinderData {
  bool laserEnabled;   ///< Включен ли лазер
  float distanceM;     ///< Расстояние в метрах
  double targetLatDeg; ///< Широта цели в градусах
  double targetLonDeg; ///< Долгота цели в градусах
};

/**
 * @brief Состояние данных с дальномера
 */
struct RangefinderDataState {
  StateMeta meta; ///< Метаинформация
  std::optional<RangefinderData> value; ///< Значение
};

/**
 * @brief Состояние тепловой палитры
 */
struct ThermalPaletteState {
  StateMeta meta; ///< Метаинформация
  std::optional<ThermalPalette> value; ///< Значение
};

/**
 * @brief Состояние усиления тепловизора
 */
struct ThermalGainState {
  StateMeta meta; ///< Метаинформация
  std::optional<ThermalGain> value; ///< Значение
};

/**
 * @brief Измерение температуры в точке
 */
struct ThermalPointMeasurement {
  uint16_t x;            ///< Координата X
  uint16_t y;            ///< Координата Y
  float temperatureC;  ///< Температура в градусах Цельсия
};

/**
 * @brief Состояние измерения температуры в точке
 */
struct ThermalPointMeasurementState {
  StateMeta meta; ///< Метаинформация
  std::optional<ThermalPointMeasurement> value; ///< Значение
};

/**
 * @brief Измерение температуры в области
 */
struct ThermalAreaMeasurement {
  uint16_t startX;   ///< Начальная координата X
  uint16_t startY;   ///< Начальная координата Y
  uint16_t endX;     ///< Конечная координата X
  uint16_t endY;     ///< Конечная координата Y
  float maxTempC;    ///< Максимальная температура в °C
  float minTempC;    ///< Минимальная температура в °C
  uint16_t maxX;     ///< Координата X максимальной температуры
  uint16_t maxY;     ///< Координата Y максимальной температуры
  uint16_t minX;     ///< Координата X минимальной температуры
  uint16_t minY;     ///< Координата Y минимальной температуры
};

/**
 * @brief Состояние измерения температуры в области
 */
struct ThermalAreaMeasurementState {
  StateMeta meta; ///< Метаинформация
  std::optional<ThermalAreaMeasurement> value; ///< Значение
};

/**
 * @brief Измерение температуры по всему кадру
 */
struct ThermalFullFrameMeasurement {
  float maxTempC; ///< Максимальная температура в °C
  float minTempC; ///< Минимальная температура в °C
  uint16_t maxX;    ///< Координата X максимальной температуры
  uint16_t maxY;    ///< Координата Y максимальной температуры
  uint16_t minX;    ///< Координата X минимальной температуры
  uint16_t minY;    ///< Координата Y минимальной температуры
};

/**
 * @brief Состояние измерения температуры по всему кадру
 */
struct ThermalFullFrameMeasurementState {
  StateMeta meta; ///< Метаинформация
  std::optional<ThermalFullFrameMeasurement> value; ///< Значение
};

/**
 * @brief Полное состояние камеры SIYI
 *
 * Объединяет все подсостояния в одну структуру
 */
struct SiyiState {
  // Device info
  HardwareIDState hardware; ///< Аппаратный ID
  FirmwareVersionState firmware; ///< Версия прошивки

  // Gimbal
  GimbalAngleState gimbalAngle; ///< Углы подвеса
  GimbalRotateSpeedState gimbalRotateSpeed; ///< Скорость вращения подвеса
  GimbalModeState gimbalMode; ///< Режим подвеса
  MountingDirectionState mountingDirection; ///< Ориетация подвеса

  // Camera (cmd 0x09, 0x0A, 0x0F, 0x12)
  GimbalConfigInfoState gimbalConfigInfo; ///< Информация о конфигурации
  FunctionFeedbackInfoState functionFeedbackInfo; ///< Обратная связь о функции
  CurrentZoomState currentZoom; ///< Текущий зум
  MaxZoomState maxZoom; ///< Максимальный зум
  GimbalCameraImageModeState imageType; ///< Тип изображения

  // Rangefinder (cmd 0x14)
  RangefinderDataState rangefinder; ///< Данные дальномера

  // Thermal (cmd 0x1A, 0x1B, temperature measurements)
  ThermalPaletteState thermalPalette; ///< Тепловая палитра
  ThermalGainState thermalGain; ///< Усиление тепловизора
  ThermalPointMeasurementState thermalPointMeasurement; ///< Измерение температуры в точке
  ThermalAreaMeasurementState thermalAreaMeasurement; ///< Измерение температуры в области
  ThermalFullFrameMeasurementState thermalFullFrameMeasurement; ///< Измерение температуры по всему кадру
};

}  // namespace siyi
