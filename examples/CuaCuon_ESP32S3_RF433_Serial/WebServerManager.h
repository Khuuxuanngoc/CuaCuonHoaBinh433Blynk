#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "RollingDoorController.h"
#include "DoorMotionTracker.h"
#include "GoogleSheetClient.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    #include <WiFi.h>
    #include <WebServer.h>
    #include <Update.h>
    typedef WebServer WebServerType;
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
    #include <Updater.h>
    typedef ESP8266WebServer WebServerType;
#endif

/**
 * @class WebServerManager
 * @brief Quản lý Web Server AP, Giao diện Web App Local đa chức năng:
 * - Màn hình khóa đăng nhập bảo mật
 * - Bảng điều khiển cửa cuốn & giám sát chuyển động
 * - Cấu hình WiFi Station & Google Apps Script Webhook
 * - Quản lý đổi mật khẩu 2 lớp (WiFi AP Pass & Master Pass)
 * - Nạp Firmware Web OTA trực tiếp qua trình duyệt
 * Hỗ trợ đồng thời cả ESP32-S3 và ESP8266.
 */
class WebServerManager {
public:
    WebServerManager(RollingDoorController& controller, DoorMotionTracker& tracker, GoogleSheetClient& sheetClient);
    ~WebServerManager();

    /**
     * @brief Khởi động WiFi AP/STA và Web Server
     */
    bool begin();

    /**
     * @brief Xử lý yêu cầu HTTP trong vòng lặp loop()
     */
    void update();

    /**
     * @brief Lấy địa chỉ IP hiện tại của ESP
     */
    IPAddress getAPIP() const;
    IPAddress getStaIP() const;
    bool isStaConnected() const;

    /**
     * @brief Lấy cấu hình hệ thống hiện tại
     */
    const SystemConfig& getConfig() const { return _config; }

    /**
     * @brief Cập nhật lại cấu hình hệ thống
     */
    void setConfig(const SystemConfig& cfg);

private:
    void setupRoutes();
    void handleRoot();
    void handleApiLogin();
    void handleApiStatus();
    void handleApiControl();
    void handleApiRfList();
    void handleApiRfLearn();
    void handleApiRfLearnStatus();
    void handleApiRfCancel();
    void handleApiRfSet();
    void handleApiRfClear();
    void handleApiRfExport();
    void handleApiRfImport();
    void handleApiWifiScan();
    void handleApiConfigWifi();
    void handleApiConfigSecurity();
    void handleApiReboot();
    void handleUpdateGet();
    void handleUpdatePost();
    void handleUpdateUpload();

    bool checkAuth();
    void handleStaConnection();

    enum StaState {
        STA_IDLE,
        STA_CONNECTING,
        STA_CONNECTED,
        STA_DISCONNECTED_PAUSED
    };

    RollingDoorController& _controller;
    DoorMotionTracker& _tracker;
    GoogleSheetClient& _sheetClient;
    WebServerType _server;
    SystemConfig _config;

    StaState _staState;
    uint32_t _staConnectStartTime;
    uint32_t _lastStaRetryTime;

    bool _otaSuccess;
    bool _otaAuthenticated;
    String _otaErrorMsg;
};

#endif // WEB_SERVER_MANAGER_H
