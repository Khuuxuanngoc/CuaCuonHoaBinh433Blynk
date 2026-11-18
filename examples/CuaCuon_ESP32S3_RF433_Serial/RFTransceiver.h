#ifndef RF_TRANSCEIVER_H
#define RF_TRANSCEIVER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include "RFProtocolTypes.h"

/**
 * @class RFTransceiver
 * @brief Đóng gói tầng giao tiếp phần cứng RF 433MHz (Thu & Phát)
 * Sử dụng thư viện chuẩn RCSwitch, hỗ trợ cả ESP32-S3 và ESP8266.
 */
class RFTransceiver {
public:
    RFTransceiver();
    ~RFTransceiver();

    /**
     * @brief Khởi tạo module thu/phát RF 433MHz
     * @param rxPin Chân ngắt nhận dữ liệu
     * @param txPin Chân phát dữ liệu
     * @param defaultTxRepeats Số lần lặp lại mặc định khi phát (mặc định: 10 lần)
     * @return true nếu khởi tạo thành công
     */
    bool begin(int8_t rxPin, int8_t txPin, uint8_t defaultTxRepeats = 10);

    /**
     * @brief Bật bộ thu nhận tín hiệu RF
     */
    void enableReceiver();

    /**
     * @brief Tắt bộ thu nhận tín hiệu RF
     */
    void disableReceiver();

    /**
     * @brief Kiểm tra xem có gói tin RF nào vừa được nhận hay không
     */
    bool isReceiverAvailable();

    /**
     * @brief Đọc lệnh RF vừa nhận được và làm sạch cờ nhận
     * @param outCmd Tham chiếu để lưu kết quả
     * @return true nếu gói tin hợp lệ (code != 0)
     */
    bool readReceivedCommand(RFCommand& outCmd);

    /**
     * @brief Xóa bộ đệm nhận của RCSwitch
     */
    void resetReceiver();

    /**
     * @brief Phát tín hiệu RF theo cấu trúc RFCommand
     * @param cmd Cấu trúc chứa thông số mã, số bit, protocol, pulselength
     * @param repeats Số lần lặp (nếu truyền 0 sẽ dùng giá trị mặc định)
     */
    void sendCommand(const RFCommand& cmd, uint8_t repeats = 0);

    void setTxRepeats(uint8_t repeats) { _txRepeats = repeats; }
    uint8_t getTxRepeats() const { return _txRepeats; }
    int8_t getRxPin() const { return _rxPin; }
    int8_t getTxPin() const { return _txPin; }
    bool isRxEnabled() const { return _isRxEnabled; }

private:
    RCSwitch _rf;
    int8_t _rxPin;
    int8_t _txPin;
    uint8_t _txRepeats;
    bool _isRxEnabled;
};

#endif // RF_TRANSCEIVER_H
