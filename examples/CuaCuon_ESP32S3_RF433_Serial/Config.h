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
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
        #define BOARD_TARGET_NAME   "ESP32-S3"
        #define BOARD_CPU_ARCH      "Xtensa LX7 Dual-Core"
    #elif defined(CONFIG_IDF_TARGET_ESP32C3)
        #define BOARD_TARGET_NAME   "ESP32-C3"
        #define BOARD_CPU_ARCH      "RISC-V Single-Core"
    #else
        #define BOARD_TARGET_NAME   "ESP32"
        #define BOARD_CPU_ARCH      "Xtensa LX6 Dual-Core"
    #endif
    #define BOARD_OPPOSITE_NAME "ESP8266 (NodeMCU)"

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    // ------------------------------------------------------------------------
    // Cấu hình chân cho ESP8266 (NodeMCU / WeMos D1 Mini)
    // ------------------------------------------------------------------------
    // D2 = GPIO 4, D1 = GPIO 5, D4 = GPIO 2 (LED_BUILTIN đảo mức logic)
    #define RF_RX_PIN           4   // D2 (GPIO 4) - Module thu RF 433MHz
    #define RF_TX_PIN           5   // D1 (GPIO 5) - Module phát RF 433MHz
    #define STATUS_LED_PIN      2   // D4 (GPIO 2 / LED_BUILTIN)
    #define BOARD_TARGET_NAME   "ESP8266 (NodeMCU)"
    #define BOARD_CPU_ARCH      "Xtensa L106 Single-Core"
    #define BOARD_OPPOSITE_NAME "ESP32-S3 / ESP32"

#else
    // ------------------------------------------------------------------------
    // Fallback cho các dòng vi điều khiển khác (AVR, STM32...)
    // ------------------------------------------------------------------------
    #define RF_RX_PIN           2
    #define RF_TX_PIN           3
    #define STATUS_LED_PIN      13
    #define BOARD_TARGET_NAME   "Generic-MCU"
    #define BOARD_CPU_ARCH      "Unknown"
    #define BOARD_OPPOSITE_NAME "ESP Boards"
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
#define EEPROM_MAGIC_KEY        0x4B59   // 'KY' kiểm tra dữ liệu EEPROM v2 hợp lệ (RF + Cấu hình)
#define EEPROM_START_ADDR       0
#define EEPROM_SIZE_BYTES       512

// ============================================================================
// NÚT KHÔI PHỤC CÀI ĐẶT GỐC (FACTORY RESET HARDWARE)
// ============================================================================
#define FACTORY_RESET_PIN       0        // Nút BOOT trên ESP32 / Nút FLASH trên ESP8266 (GPIO 0)
#define FACTORY_RESET_HOLD_MS   5000     // Giữ nút 5 giây để khôi phục mặc định ban đầu

// ============================================================================
// MÃ LỆNH CẤU HÌNH SẴN (PRESET / DEFAULT)
// Dùng để nạp trực tiếp mã đã học từ ESP32-S3 sang ESP8266 mà không cần remote.
// Để 0x00000000 nếu muốn học lệnh động qua remote.
// ============================================================================
#define PRESET_CODE_UP          0x00000000
#define PRESET_CODE_DOWN        0x00000000
#define PRESET_CODE_STOP        0x00000000
#define PRESET_CODE_LOCK        0x00000000
#define PRESET_BIT_LENGTH       24
#define PRESET_PROTOCOL         1
#define PRESET_PULSE_LENGTH     350

// ============================================================================
// CẤU HÌNH THỜI GIAN HÀNH TRÌNH CỬA (VIRTUAL MOTION TRACKER)
// ============================================================================
#define DEFAULT_TRAVEL_TIME_SEC 25       // Thời gian cửa cuốn chạy hết từ sàn lên nóc (giây)

// ============================================================================
// CẤU HÌNH MẠNG WIFI & WEBSERVER
// ============================================================================
// Chế độ phát WiFi riêng (Access Point):
#define WIFI_AP_SSID            "CuaCuon_HoaBinh"  // Tên WiFi phát ra từ ESP
#define WIFI_AP_PASS            "12345678"         // Mật khẩu WiFi (tối thiểu 8 ký tự)

// Chế độ kết nối WiFi gia đình (Station - Để gửi dữ liệu lên Google Sheets):
#define WIFI_STA_SSID           ""                 // Nhập tên WiFi nhà bạn (để trống nếu chưa dùng)
#define WIFI_STA_PASS           ""                 // Mật khẩu WiFi nhà bạn

// Mật khẩu bảo vệ Web App trên ESP:
#define WEB_APP_PASSWORD        "123456"

// ============================================================================
// CẤU HÌNH GOOGLE APPS SCRIPT WEBHOOK
// ============================================================================
// Dán URL Web App của bạn từ Google Apps Script (bước triển khai):
#define GAS_WEBHOOK_URL         ""
#define GAS_WEBHOOK_TOKEN       "HoaBinh_RollingDoor_2026"

#endif // CONFIG_H
