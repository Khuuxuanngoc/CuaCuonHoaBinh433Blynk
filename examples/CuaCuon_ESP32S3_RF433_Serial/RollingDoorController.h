#ifndef ROLLING_DOOR_CONTROLLER_H
#define ROLLING_DOOR_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"
#include "RFProtocolTypes.h"
#include "RFTransceiver.h"
#include "StorageManager.h"

/**
 * @brief Định nghĩa các hàm Callback thông báo sự kiện (Event-driven)
 */
typedef void (*OnLearnCallback)(DoorAction action, const RFCommand& cmd, bool success);
typedef void (*OnActionCallback)(DoorAction action, const RFCommand& cmd);
typedef void (*OnSniffCallback)(const RFCommand& cmd);
typedef void (*OnStateChangeCallback)(ControllerState state, const char* message);

/**
 * @class IRollingDoor
 * @brief Giao diện trừu tượng (Interface) cho hệ thống điều khiển cửa cuốn
 * Cho phép kế thừa, mở rộng hoặc mock test cho các dự án chuyên nghiệp.
 */
class IRollingDoor {
public:
    virtual ~IRollingDoor() {}
    virtual bool begin(int8_t rxPin = RF_RX_PIN, int8_t txPin = RF_TX_PIN, int8_t ledPin = STATUS_LED_PIN) = 0;
    virtual void update() = 0;
    virtual bool startLearning(DoorAction action, uint32_t timeoutMs = DEFAULT_LEARN_TIMEOUT) = 0;
    virtual void cancelLearning() = 0;
    virtual bool isLearning() const = 0;
    virtual bool sendCommand(DoorAction action) = 0;
    virtual bool save() = 0;
    virtual bool load() = 0;
    virtual bool clear() = 0;
    virtual const RFCommand& getCommand(DoorAction action) const = 0;
};

/**
 * @class RollingDoorController
 * @brief Lớp điều khiển chính độc lập với giao tiếp ngoại vi (Serial, Web, Blynk)
 * Đảm nhận State Machine, học lệnh, phát lệnh, và quản lý bộ nhớ.
 */
class RollingDoorController : public IRollingDoor {
public:
    RollingDoorController();
    virtual ~RollingDoorController();

    /**
     * @brief Khởi tạo phần cứng và tải dữ liệu đã lưu
     */
    virtual bool begin(int8_t rxPin = RF_RX_PIN, int8_t txPin = RF_TX_PIN, int8_t ledPin = STATUS_LED_PIN) override;

    /**
     * @brief Hàm cập nhật vòng lặp chính (Non-blocking, gọi trong loop())
     */
    virtual void update() override;

    /**
     * @brief Bắt đầu học lệnh cho một nút xác định
     * @param action Nút cần học (ACTION_UP, ACTION_DOWN, ACTION_STOP, ACTION_LOCK)
     * @param timeoutMs Thời gian tối đa chờ remote (mặc định 15000ms)
     * @return true nếu bắt đầu chế độ học thành công
     */
    virtual bool startLearning(DoorAction action, uint32_t timeoutMs = DEFAULT_LEARN_TIMEOUT) override;

    /**
     * @brief Hủy chế độ học lệnh hiện tại
     */
    virtual void cancelLearning() override;

    /**
     * @brief Kiểm tra xem hệ thống có đang trong chế độ học lệnh hay không
     */
    virtual bool isLearning() const override { return _state == STATE_LEARNING; }

    /**
     * @brief Lấy nút hiện đang được học
     */
    DoorAction getCurrentLearningAction() const { return _learnAction; }

    /**
     * @brief Lấy thời gian chờ học còn lại (milliseconds)
     */
    uint32_t getLearningRemainingMs() const;

    /**
     * @brief Phát tín hiệu điều khiển cửa tương ứng với hành động
     * @param action Nút cần phát
     * @return true nếu phát thành công (lệnh hợp lệ)
     */
    virtual bool sendCommand(DoorAction action) override;

    /**
     * @brief Phát trực tiếp một lệnh RF tùy biến
     */
    bool sendRawCommand(const RFCommand& cmd, uint8_t repeats = 0);

    /**
     * @brief Lưu các lệnh hiện tại vào bộ nhớ Flash
     */
    virtual bool save() override;

    /**
     * @brief Tải lại các lệnh từ bộ nhớ Flash
     */
    virtual bool load() override;

    /**
     * @brief Xóa toàn bộ lệnh trong RAM và Flash
     */
    virtual bool clear() override;

    /**
     * @brief Lấy thông tin lệnh của một nút
     */
    virtual const RFCommand& getCommand(DoorAction action) const override;

    /**
     * @brief Thiết lập trực tiếp lệnh cho một nút (VD: nhập bằng tay từ giao diện Web)
     */
    bool setCommand(DoorAction action, const RFCommand& cmd);

    /**
     * @brief Kiểm tra xem một nút đã có lệnh hợp lệ hay chưa
     */
    bool hasValidCommand(DoorAction action) const;

    /**
     * @brief Lấy trạng thái hoạt động hiện tại
     */
    ControllerState getState() const { return _state; }

    /**
     * @brief Bật / Tắt chế độ Sniffer (Monitor mọi sóng RF 433 bắt được)
     */
    void setSnifferMode(bool enabled);
    bool isSnifferMode() const { return _snifferMode; }

    /**
     * @brief Kiểm tra xem có thay đổi nào chưa lưu hay không
     */
    bool isModified() const { return _storage.isModified(); }

    // Đăng ký Callback sự kiện
    void setOnLearnCallback(OnLearnCallback cb) { _onLearnCb = cb; }
    void setOnActionCallback(OnActionCallback cb) { _onActionCb = cb; }
    void setOnSniffCallback(OnSniffCallback cb) { _onSniffCb = cb; }
    void setOnStateChangeCallback(OnStateChangeCallback cb) { _onStateChangeCb = cb; }

private:
    void setState(ControllerState newState, const char* reason = nullptr);
    void updateLed();

    RFTransceiver _rf;
    StorageManager _storage;

    RFCommand _commands[ACTION_COUNT];
    ControllerState _state;
    DoorAction _learnAction;

    uint32_t _learnStartTime;
    uint32_t _learnTimeoutMs;
    uint32_t _lastCaptureTime;

    int8_t _ledPin;
    uint32_t _lastLedToggleTime;
    bool _ledState;

    bool _snifferMode;

    OnLearnCallback _onLearnCb;
    OnActionCallback _onActionCb;
    OnSniffCallback _onSniffCb;
    OnStateChangeCallback _onStateChangeCb;
};

#endif // ROLLING_DOOR_CONTROLLER_H
