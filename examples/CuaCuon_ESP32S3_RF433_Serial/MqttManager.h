#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include "Config.h"
#include "StorageManager.h"
#include "RFProtocolTypes.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    #include <WiFi.h>
    #include <WiFiClientSecure.h>
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <WiFiClientSecure.h>
#endif

class RollingDoorController;
class DoorMotionTracker;

/**
 * @class MqttManager
 * @brief Quản lý kết nối MQTT hai chiều (điều khiển tức thì < 0.2s qua 4G/Internet)
 * - Tự động hỗ trợ cả cổng 1883 (TCP thường) và cổng 8883 (SSL/TLS bảo mật)
 * - Cơ chế Non-blocking State Machine, không bao giờ làm treo loop() hay sóng AP
 * - Đăng ký nhận lệnh từ: <prefix>/command ("up", "down", "stop", "lock")
 * - Báo cáo trạng thái theo thời gian thực lên: <prefix>/state (JSON)
 * - Báo cáo trạng thái sống (LWT - Last Will & Testament) lên: <prefix>/status ("online" / "offline")
 */
class MqttManager {
public:
    MqttManager(RollingDoorController& controller, DoorMotionTracker& tracker);
    ~MqttManager();

    /**
     * @brief Khởi tạo module MQTT với cấu hình hệ thống
     */
    bool begin(const SystemConfig& config);

    /**
     * @brief Vòng lặp cập nhật trong loop()
     */
    void update();

    /**
     * @brief Cập nhật cấu hình mới khi người dùng đổi cài đặt trên Web AP
     */
    void setConfig(const SystemConfig& config);

    /**
     * @brief Phát trạng thái cửa hiện tại lên MQTT Topic <prefix>/state
     */
    void publishState(const char* source = nullptr);

    /**
     * @brief Kiểm tra xem MQTT có đang kết nối thành công không
     */
    bool isConnected();

    /**
     * @brief Kiểm tra xem MQTT có đang được kích hoạt trong cài đặt không
     */
    bool isEnabled() const { return _config.mqttEnabled; }

private:
    void setupClient();
    void tryReconnect();
    void onMessageReceived(char* topic, uint8_t* payload, unsigned int length);

    RollingDoorController& _controller;
    DoorMotionTracker& _tracker;
    SystemConfig _config;

    WiFiClient _tcpClient;
    WiFiClientSecure _tlsClient;
    PubSubClient _mqtt;

    uint32_t _lastReconnectAttempt;
    bool _isTls;
};

#endif // MQTT_MANAGER_H
