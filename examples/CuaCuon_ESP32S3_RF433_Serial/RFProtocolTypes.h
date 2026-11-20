#ifndef RF_PROTOCOL_TYPES_H
#define RF_PROTOCOL_TYPES_H

#include <Arduino.h>

/**
 * @brief Định nghĩa 4 trạng thái / hành động của cửa cuốn
 */
enum DoorAction : uint8_t {
    ACTION_UP    = 0,  // Nút Lên (Mở cửa)
    ACTION_DOWN  = 1,  // Nút Xuống (Đóng cửa)
    ACTION_STOP  = 2,  // Nút Dừng (Tạm dừng)
    ACTION_LOCK  = 3,  // Nút Khóa (Khóa cửa / Khóa hộp nhận)
    ACTION_COUNT = 4   // Tổng số nút
};

/**
 * @brief Cấu trúc dữ liệu chứa trọn vẹn thông số một khung truyền RF 433MHz
 * Phù hợp với chuẩn giải mã và phát của thư viện RCSwitch.
 */
struct RFCommand {
    uint32_t code;         // Giá trị mã RF (32-bit integer)
    uint8_t  bitLength;    // Độ dài bit (thường 24 bit)
    uint8_t  protocol;     // Giao thức (1 đến 12 trong RCSwitch)
    uint16_t pulseLength;  // Độ rộng xung (microseconds, VD: 350us)
    bool     isValid;      // Đã học lệnh hay chưa

    RFCommand()
        : code(0), bitLength(0), protocol(1), pulseLength(350), isValid(false) {}

    RFCommand(uint32_t c, uint8_t b, uint8_t p, uint16_t pulse)
        : code(c), bitLength(b), protocol(p), pulseLength(pulse), isValid(true) {}
};

/**
 * @brief Trạng thái hoạt động của bộ điều khiển
 */
enum ControllerState : uint8_t {
    STATE_IDLE = 0,        // Chờ lệnh bình thường
    STATE_LEARNING,        // Đang chờ bắt sóng RF để học
    STATE_TRANSMITTING     // Đang phát tín hiệu RF
};

/**
 * @brief Lấy tên viết tắt tiếng Anh của hành động
 */
inline const char* getActionName(DoorAction action) {
    switch (action) {
        case ACTION_UP:    return "UP";
        case ACTION_DOWN:  return "DOWN";
        case ACTION_STOP:  return "STOP";
        case ACTION_LOCK:  return "LOCK";
        default:           return "UNKNOWN";
    }
}

/**
 * @brief Lấy mô tả tiếng Việt của hành động (có dấu cho Web App)
 */
inline const char* getActionDescVi(DoorAction action) {
    switch (action) {
        case ACTION_UP:    return "Lên (Mở cửa)";
        case ACTION_DOWN:  return "Xuống (Đóng cửa)";
        case ACTION_STOP:  return "Dừng (Tạm dừng)";
        case ACTION_LOCK:  return "Khóa (Khóa cửa)";
        default:           return "Không xác định";
    }
}

/**
 * @brief Lấy mô tả tiếng Việt không dấu (cho Serial Monitor tránh lỗi font)
 */
inline const char* getActionDescNoAccent(DoorAction action) {
    switch (action) {
        case ACTION_UP:    return "Len (Mo cua)";
        case ACTION_DOWN:  return "Xuong (Dong cua)";
        case ACTION_STOP:  return "Dung (Tam dung)";
        case ACTION_LOCK:  return "Khoa (Khoa cua)";
        default:           return "Khong xac dinh";
    }
}

/**
 * @brief Parse chuỗi ký tự thành DoorAction tương ứng
 * Hỗ trợ cả tiếng Anh (up, down, stop, lock) và tiếng Việt (len, xuong, dung, khoa) hoặc số (1, 2, 3, 4)
 * @return true nếu parse thành công, gán giá trị vào outAction
 */
inline bool parseAction(const char* str, DoorAction& outAction) {
    if (!str || strlen(str) == 0) return false;

    if (strcasecmp(str, "up") == 0 || strcasecmp(str, "len") == 0 || strcmp(str, "1") == 0) {
        outAction = ACTION_UP;
        return true;
    }
    if (strcasecmp(str, "down") == 0 || strcasecmp(str, "xuong") == 0 || strcmp(str, "2") == 0) {
        outAction = ACTION_DOWN;
        return true;
    }
    if (strcasecmp(str, "stop") == 0 || strcasecmp(str, "dung") == 0 || strcmp(str, "3") == 0) {
        outAction = ACTION_STOP;
        return true;
    }
    if (strcasecmp(str, "lock") == 0 || strcasecmp(str, "khoa") == 0 || strcmp(str, "4") == 0) {
        outAction = ACTION_LOCK;
        return true;
    }
    return false;
}

#endif // RF_PROTOCOL_TYPES_H
