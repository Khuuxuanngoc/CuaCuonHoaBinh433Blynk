#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "RFProtocolTypes.h"

/**
 * @class StorageManager
 * @brief Quản lý lưu trữ thông số mã lệnh vào bộ nhớ Non-Volatile Flash
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
     * @brief Xóa trắng dữ liệu đã lưu trong bộ nhớ
     */
    bool clear();

    /**
     * @brief Kiểm tra trạng thái đã có dữ liệu lưu trước đó hay chưa
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
        uint16_t checksum;
    };
    uint16_t calculateChecksum(const EEPROMRecord& record);
#endif
};

#endif // STORAGE_MANAGER_H
