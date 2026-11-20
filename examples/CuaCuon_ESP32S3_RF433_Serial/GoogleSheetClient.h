#ifndef GOOGLE_SHEET_CLIENT_H
#define GOOGLE_SHEET_CLIENT_H

#include <Arduino.h>
#include "Config.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    #include <WiFi.h>
    #include <WiFiClientSecure.h>
    #include <HTTPClient.h>
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <WiFiClientSecure.h>
    #include <ESP8266HTTPClient.h>
#endif

class RollingDoorController;

/**
 * @class GoogleSheetClient
 * @brief Gửi sự kiện đóng/mở cửa lên Google Apps Script Webhook
 * Tự động chuyển hướng (Redirect 302), kết nối HTTPS an toàn (Insecure SSL),
 * Hàng đợi sự kiện không gây chặn vòng lặp chính.
 */
class GoogleSheetClient {
public:
    GoogleSheetClient();
    ~GoogleSheetClient();

    /**
     * @brief Khởi động client
     */
    bool begin(const char* webhookUrl = GAS_WEBHOOK_URL, const char* token = GAS_WEBHOOK_TOKEN);

    /**
     * @brief Gán controller để tự động thực thi lệnh từ xa và đồng bộ mã RF
     */
    void setController(RollingDoorController* controller) { _controller = controller; }

    /**
     * @brief Vòng lặp kiểm tra và gửi sự kiện đang chờ (gọi trong loop())
     */
    void update();

    /**
     * @brief Đặt sự kiện vào hàng đợi để gửi lên Google Sheet
     * @param event Tên sự kiện (MỞ CỬA, ĐÓNG CỬA, TẠM DỪNG, KHÓA...)
     * @param doorState Trạng thái cửa
     * @param openDurationSec Thời gian mở cửa (giây)
     * @param source Nguồn kích hoạt (Remote RF, Web Local...)
     */
    void sendEvent(const char* event, const char* doorState, uint32_t openDurationSec, const char* source);

    /**
     * @brief Gửi thông báo khởi động (Có điện lại sau khi mất nguồn)
     */
    void sendBootNotice(const char* reason = "Power-on / Reset");

    /**
     * @brief Gửi gói Heartbeat định kỳ để Google biết ESP vẫn Online
     */
    void sendHeartbeat();

    bool isConfigured() const { return strlen(_webhookUrl) > 10; }

private:
    void executePost();
    void handleServerResponse(const String& payload);

    RollingDoorController* _controller;

    char _webhookUrl[256];
    char _token[40];

    struct PendingEvent {
        char event[24];
        char doorState[32];
        uint32_t durationSec;
        char source[24];
        bool isPending;
    } _pending;

    uint32_t _lastAttemptTime;
    uint32_t _lastPingTime;
    bool _bootNoticeSent;
};

#endif // GOOGLE_SHEET_CLIENT_H
