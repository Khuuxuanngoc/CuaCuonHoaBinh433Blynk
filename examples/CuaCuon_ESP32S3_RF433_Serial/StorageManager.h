#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "RFProtocolTypes.h"

/**
 * @struct SystemConfig
 * @brief Cấu hình hệ thống WiFi, Mật khẩu và Dịch vụ Cloud
 */
struct SystemConfig {
    char wifiSSID[33];       // Tên WiFi nhà (Station)
    char wifiPass[65];       // Mật khẩu WiFi nhà
    char gasWebhookUrl[160]; // URL Google Apps Script Webhook
    char webPassword[33];    // Mật khẩu Quản trị Master (Bảng điều khiển & Cài đặt)
    char apPassword[33];     // Mật khẩu WiFi AP phát ra từ ESP (tối thiểu 8 ký tự)
    uint16_t travelTimeSec;  // Thời gian hành trình cửa (giây)
};

/**
 * @class StorageManager
 * @brief Quản lý lưu trữ thông số mã lệnh và cấu hình hệ thống vào bộ nhớ Non-Volatile Flash
 * Hỗ trợ tự động:
 * - Preferences (NVS) trên ESP32 / ESP32-S3
 * - EEPROM Flash Sector trên ESP8266
 */
class StorageManager {
public:
    StorageManager();
    ~StorageManager();

    /**
     * @brief Khởi tạo hệ thống lưu trữ
     */
    bool begin();

    /**
     * @brief Lưu toàn bộ danh sách 4 lệnh vào bộ nhớ Flash
     * @param commands Mảng 4 phần tử RFCommand
     * @return true nếu lưu thành công
     */
    bool saveAll(const RFCommand commands[ACTION_COUNT]);

    /**
     * @brief Đọc toàn bộ danh sách 4 lệnh từ bộ nhớ Flash
     * @param commands Mảng 4 phần tử RFCommand nhận kết quả
     * @return true nếu đọc thành công và dữ liệu hợp lệ
     */
    bool loadAll(RFCommand commands[ACTION_COUNT]);

    /**
     * @brief Lưu riêng lẻ 1 lệnh
     */
    bool saveCommand(DoorAction action, const RFCommand& cmd);

    /**
     * @brief Đọc riêng lẻ 1 lệnh
     */
    bool loadCommand(DoorAction action, RFCommand& cmd);

    /**
     * @brief Xóa trắng mã lệnh RF đã lưu trong bộ nhớ
     */
    bool clear();

    /**
     * @brief Lưu cấu hình hệ thống (WiFi, Pass, Webhook...)
     */
    bool saveSystemConfig(const SystemConfig& config);

    /**
     * @brief Đọc cấu hình hệ thống từ Flash
     */
    bool loadSystemConfig(SystemConfig& config);

    /**
     * @brief Lấy cấu hình mặc định từ Config.h
     */
    void getDefaultSystemConfig(SystemConfig& config);

    /**
     * @brief Khôi phục toàn bộ cài đặt gốc (xóa mã RF + reset cấu hình về mặc định)
     */
    bool resetToFactoryDefaults();

    /**
     * @brief Kiểm tra trạng thái đã có dữ liệu RF lưu trước đó hay chưa
     */
    bool hasSavedData() const { return _hasSavedData; }

    /**
     * @brief Kiểm tra xem có thay đổi nào chưa được lưu vào Flash hay không
     */
    bool isModified() const { return _isModified; }
    void setModified(bool modified) { _isModified = modified; }

private:
    bool _hasSavedData;
    bool _isModified;

#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    // Cấu trúc gói dữ liệu lưu vào EEPROM trên ESP8266
    struct EEPROMRecord {
        uint16_t magic;
        RFCommand commands[ACTION_COUNT];
        SystemConfig config;
        uint16_t checksum;
    };
    uint16_t calculateChecksum(const EEPROMRecord& record);
#endif
};

#endif // STORAGE_MANAGER_H
