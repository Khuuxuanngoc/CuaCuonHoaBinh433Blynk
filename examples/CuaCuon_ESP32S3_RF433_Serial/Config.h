#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/**
 * ============================================================================
 * CẤU HÌNH PHẦN CỨNG & THAM SỐ HỆ THỐNG
 * ============================================================================
 * Tự động nhận diện ESP32 / ESP32-S3 hoặc ESP8266 để cấu hình chân phù hợp.
 */

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    // ------------------------------------------------------------------------
    // Cấu hình chân cho ESP32-S3 (Sử dụng kênh qua FET chuyển mức logic 5V <-> 3.3V)
    #define RF_RX_PIN           8   // Chân Data module thu RF 433MHz (qua FET hạ áp 5V -> 3.3V an toàn)
    #define RF_TX_PIN           9   // Chân Data module phát RF 433MHz (qua FET kích áp 3.3V -> 5V công suất cao)
    #define STATUS_LED_PIN      2   // Chân LED báo trạng thái (nhấp nháy khi học/phát)
    #define BOARD_TARGET_NAME   "ESP32-S3"

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    // ------------------------------------------------------------------------
    // Cấu hình chân cho ESP8266 (NodeMCU / WeMos D1 Mini)
    // ------------------------------------------------------------------------
    // D2 = GPIO 4, D1 = GPIO 5, D4 = GPIO 2 (LED_BUILTIN đảo mức logic)
    #define RF_RX_PIN           4   // D2 (GPIO 4) - Module thu RF 433MHz
    #define RF_TX_PIN           5   // D1 (GPIO 5) - Module phát RF 433MHz
    #define STATUS_LED_PIN      2   // D4 (GPIO 2 / LED_BUILTIN)
    #define BOARD_TARGET_NAME   "ESP8266"

#else
    // ------------------------------------------------------------------------
    // Fallback cho các dòng vi điều khiển khác (AVR, STM32...)
    // ------------------------------------------------------------------------
    #define RF_RX_PIN           2
    #define RF_TX_PIN           3
    #define STATUS_LED_PIN      13
    #define BOARD_TARGET_NAME   "Generic-MCU"
#endif

// ============================================================================
// THAM SỐ TRUYỀN THÔNG & THỜI GIAN
// ============================================================================
#define SERIAL_BAUD_RATE        115200   // Tốc độ truyền Serial Monitor
#define DEFAULT_LEARN_TIMEOUT   15000    // Thời gian chờ học lệnh mặc định (15 giây)
#define DEFAULT_TX_REPEATS      10       // Số lần lặp lại gói tin khi phát RF (đảm bảo độ nhạy)
#define DEBOUNCE_LEARN_MS       350      // Khoảng thời gian chống trùng gói tin khi học (ms)

// ============================================================================
// CẤU HÌNH BỘ NHỚ LƯU TRỮ
// ============================================================================
// ESP32 sử dụng Preferences (NVS Flash), ESP8266 sử dụng EEPROM Sector
#define NVS_NAMESPACE           "cuacuon"
#define EEPROM_MAGIC_KEY        0x4B58   // 'KX' kiểm tra dữ liệu EEPROM hợp lệ
#define EEPROM_START_ADDR       0
#define EEPROM_SIZE_BYTES       512

#endif // CONFIG_H
