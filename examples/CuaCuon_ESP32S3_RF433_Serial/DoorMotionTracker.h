#ifndef DOOR_MOTION_TRACKER_H
#define DOOR_MOTION_TRACKER_H

#include <Arduino.h>
#include "Config.h"
#include "RFProtocolTypes.h"

/**
 * @brief Các trạng thái chuyển động ảo của cửa cuốn (không dùng cảm biến cơ học)
 */
enum MotionState : uint8_t {
    MOTION_CLOSED = 0,    // Cửa đã đóng kín hoàn toàn (0%)
    MOTION_OPENING,       // Cửa đang mở lên...
    MOTION_OPENED,        // Cửa đã mở hoàn toàn (100%)
    MOTION_CLOSING,       // Cửa đang đóng xuống...
    MOTION_STOPPED,       // Cửa đang tạm dừng lơ lửng giữa chừng
    MOTION_LOCKED         // Hộp điều khiển đang bị khóa
};

typedef void (*OnMotionChangeCallback)(MotionState state, const char* stateName, uint32_t openDurationSec, const char* source);

/**
 * @class DoorMotionTracker
 * @brief Bộ ước lượng trạng thái và giám sát thời gian đóng/mở cửa cuốn qua sóng RF
 * Tính toán thời gian mở, hành trình, và % độ mở mà không cần lắp cảm biến vật lý.
 */
class DoorMotionTracker {
public:
    DoorMotionTracker();
    ~DoorMotionTracker();

    /**
     * @brief Khởi tạo bộ giám sát hành trình
     * @param travelTimeSec Thời gian cửa chạy hết hành trình (giây, mặc định 25s)
     */
    bool begin(uint16_t travelTimeSec = DEFAULT_TRAVEL_TIME_SEC);

    /**
     * @brief Hàm cập nhật vòng lặp chính (gọi trong loop())
     */
    void update();

    /**
     * @brief Xử lý khi có lệnh RF hoặc lệnh Web được kích hoạt
     * @param action Hành động (Lên, Xuống, Dừng, Khóa)
     * @param source Nguồn kích hoạt (Remote RF, Web Local, AppScript...)
     */
    void triggerAction(DoorAction action, const char* source = "Remote RF");

    MotionState getState() const { return _state; }
    const char* getStateNameVi() const;
    uint8_t getOpenPercentage() const { return _currentPercentage; }
    
    // Thời gian cửa đang mở liên tục (giây)
    uint32_t getCurrentOpenDurationSec() const;
    
    // Thời gian mở của chu kỳ gần nhất khi đã đóng (giây)
    uint32_t getLastTotalOpenDurationSec() const { return _lastTotalOpenDurationSec; }

    uint16_t getTravelTimeSec() const { return _travelTimeSec; }
    void setTravelTimeSec(uint16_t sec);

    const char* getLastSource() const { return _lastSource; }
    uint32_t getLastActionTimestamp() const { return _lastActionTime; }

    void setOnMotionChangeCallback(OnMotionChangeCallback cb) { _onMotionChangeCb = cb; }

private:
    void setState(MotionState newState, const char* source);

    MotionState _state;
    uint16_t _travelTimeSec;
    uint8_t _currentPercentage;       // 0% (Đóng kín) đến 100% (Mở hết)
    
    uint32_t _travelStartTime;        // Thời điểm bắt đầu chạy hành trình
    uint32_t _openCycleStartTime;     // Thời điểm cửa bắt đầu mở
    uint32_t _lastTotalOpenDurationSec;
    uint32_t _lastActionTime;

    char _lastSource[20];
    OnMotionChangeCallback _onMotionChangeCb;
};

#endif // DOOR_MOTION_TRACKER_H
