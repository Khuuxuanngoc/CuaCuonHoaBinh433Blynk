/**
 * ============================================================================
 * Dự án   : Điều Khiển Cửa Cuốn RF 433MHz - Phiên Bản 1.0 (Serial CLI & Web App)
 * Nền tảng: ESP32-S3 (Sẵn sàng Port sang ESP8266)
 * Tác giả : Hệ thống Robotics & IoT Hòa Bình
 * Chuẩn   : Tuân thủ AI_WORKSPACE.md & Kiến trúc Hướng Đối Tượng (OOP)
 * ============================================================================
 */

#include "Config.h"
#include "RollingDoorController.h"
#include "DoorMotionTracker.h"
#include "WebServerManager.h"
#include "GoogleSheetClient.h"
#include "SerialCLI.h"
#include "MqttManager.h"

// Khởi tạo các đối tượng điều khiển theo kiến trúc OOP
RollingDoorController doorController;
DoorMotionTracker     motionTracker;
GoogleSheetClient     sheetClient;
MqttManager           mqttManager(doorController, motionTracker);
WebServerManager      webServer(doorController, motionTracker, sheetClient);
SerialCLI             serialCli(doorController);

// Biến theo dõi giữ nút BOOT để Factory Reset
static uint32_t s_bootPressStartTime = 0;

// Callback khi module thu bắt được sóng từ Remote RF cầm tay ngoài không khí
void onRemoteHeard(DoorAction action, const RFCommand& cmd) {
    (void)cmd;
    motionTracker.triggerAction(action, "Remote RF");
}

// Helper tra cứu trạng thái không dấu cho Serial Monitor (tránh lỗi font terminal)
const char* getMotionStateNameNoAccent(MotionState state) {
    switch (state) {
        case MOTION_CLOSED:   return "DA DONG KIN";
        case MOTION_OPENING:  return "DANG MO LEN...";
        case MOTION_OPENED:   return "DA MO HOAN TOAN";
        case MOTION_CLOSING:  return "DANG DONG XUONG...";
        case MOTION_STOPPED:  return "TAM DUNG LO LUNG";
        case MOTION_LOCKED:   return "DANG KHOA HOP";
        default:              return "KHONG XAC DINH";
    }
}

// Callback khi trạng thái chuyển động của cửa thay đổi
void onMotionChanged(MotionState state, const char* stateName, uint32_t openDurationSec, const char* source) {
    Serial.print(F("[GIAM SAT HANH TRINH] "));
    Serial.print(getMotionStateNameNoAccent(state));
    Serial.print(F(" | Kich hoat boi: "));
    Serial.println(source);

    // Bắn dữ liệu lên Google Apps Script Webhook nếu có cấu hình
    if (sheetClient.isConfigured()) {
        const char* evt = (state == MOTION_OPENING || state == MOTION_OPENED) ? "MỞ CỬA" :
                          (state == MOTION_CLOSING || state == MOTION_CLOSED) ? "ĐÓNG CỬA" :
                          (state == MOTION_STOPPED) ? "TẠM DỪNG" : "KHÓA HỘP";
        sheetClient.sendEvent(evt, stateName, openDurationSec, source);
    }

    // Báo cáo trạng thái tức thời qua MQTT Cloud nếu đang kết nối
    mqttManager.publishState(source);
}

// Xử lý kiểm tra nút BOOT / FLASH để khôi phục cài đặt gốc
void checkFactoryResetButton() {
    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        if (s_bootPressStartTime == 0) {
            s_bootPressStartTime = millis();
        } else if (millis() - s_bootPressStartTime >= FACTORY_RESET_HOLD_MS) {
            Serial.println();
            Serial.println(F("**********************************************************"));
            Serial.println(F("[FACTORY RESET] Nut BOOT duoc giu 5 giay!"));
            Serial.println(F("Dang xoa Flash va khoi phuc toan bo cai dat goc..."));
            Serial.println(F("**********************************************************"));

            doorController.getStorage().resetToFactoryDefaults();
            doorController.clear();

            // Nhấp nháy đèn LED liên tục báo hiệu reset thành công
            for (uint8_t i = 0; i < 10; i++) {
                digitalWrite(STATUS_LED_PIN, HIGH);
                delay(80);
                digitalWrite(STATUS_LED_PIN, LOW);
                delay(80);
            }

            Serial.println(F("[HOAN TAT] He thong dang khoi dong lai..."));
            ESP.restart();
        }
    } else {
        s_bootPressStartTime = 0;
    }
}

void setup() {
    // 1. Đảm bảo chân phát RF ở mức LOW ngay khi khởi động (chống treo sóng do trở pull-up của FET)
    pinMode(RF_TX_PIN, OUTPUT);
    digitalWrite(RF_TX_PIN, LOW);

    // Cấu hình chân nút BOOT / Factory Reset
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

    // Khởi tạo giao tiếp Serial Monitor
    Serial.begin(SERIAL_BAUD_RATE);

    // Chờ cổng Serial khả dụng (đặc biệt cần thiết cho native USB trên ESP32-S3)
    uint32_t startWait = millis();
    while (!Serial && (millis() - startWait < 2000)) {
        delay(10);
    }

    Serial.println();
    Serial.println(F("=========================================================="));
    Serial.println(F("   HE THONG DIEU KHIEN CUA CUON RF 433MHz - PHIEN BAN V2  "));
    Serial.print(F("   Nen tang vi dieu khien: "));
    Serial.println(F(BOARD_TARGET_NAME));
    Serial.println(F("=========================================================="));

    // Kiểm tra nếu đang đè nút BOOT lúc vừa cắm nguồn
    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        Serial.println(F("[BOOT] Phat hien nhan giu nut BOOT..."));
        uint32_t t0 = millis();
        bool doReset = false;
        while (digitalRead(FACTORY_RESET_PIN) == LOW) {
            delay(50);
            if (millis() - t0 >= FACTORY_RESET_HOLD_MS) {
                doReset = true;
                break;
            }
        }
        if (doReset) {
            Serial.println(F("[FACTORY RESET] Khoi phuc cai dat goc khi khoi dong!"));
            doorController.getStorage().resetToFactoryDefaults();
            doorController.clear();
            for (uint8_t i = 0; i < 10; i++) {
                digitalWrite(STATUS_LED_PIN, HIGH); delay(60);
                digitalWrite(STATUS_LED_PIN, LOW); delay(60);
            }
            ESP.restart();
        }
    }

    // 2. Khởi tạo bộ điều khiển trung tâm với cấu hình chân từ Config.h
    if (!doorController.begin(RF_RX_PIN, RF_TX_PIN, STATUS_LED_PIN)) {
        Serial.println(F("[LOI KHOI TAO] Khong the khoi dong RollingDoorController!"));
    }

    // Đọc cấu hình hệ thống đã lưu
    SystemConfig cfg;
    doorController.getStorage().loadSystemConfig(cfg);

    // 3. Khởi tạo bộ ước lượng chuyển động ảo & đo thời gian mở (không dùng cảm biến)
    motionTracker.begin(cfg.travelTimeSec > 0 ? cfg.travelTimeSec : DEFAULT_TRAVEL_TIME_SEC);
    motionTracker.setOnMotionChangeCallback(onMotionChanged);

    // Đăng ký bộ lắng nghe sóng RF cầm tay ngoài không khí
    doorController.setOnRemoteReceivedCallback(onRemoteHeard);

    // 4. Khởi tạo kết nối Google Sheet Client
    sheetClient.setController(&doorController);
    sheetClient.begin(cfg.gasWebhookUrl, GAS_WEBHOOK_TOKEN);

    // 5. Khởi tạo MQTT Manager (Điều khiển tức thì < 0.2s)
    mqttManager.begin(cfg);

    // 6. Khởi tạo WebServer AP và Web App Local
    webServer.setMqttManager(&mqttManager);
    webServer.begin();

    // 7. Khởi tạo giao diện dòng lệnh Serial CLI
    serialCli.begin();

    Serial.println(F("----------------------------------------------------------"));
    Serial.print(F("WiFi AP phat ra : ")); Serial.print(F(WIFI_AP_SSID));
    Serial.print(F(" | Pass: ")); Serial.println(cfg.apPassword);
    Serial.print(F("Master Password : ")); Serial.println(cfg.webPassword);
    Serial.print(F("Dia chi Web App Local : http://")); Serial.println(webServer.getAPIP());
    Serial.println(F("Trang nap Firmware OTA: http://192.168.4.1/update"));
    Serial.println(F("----------------------------------------------------------"));
}

void loop() {
    // Cập nhật State Machine và xử lý sự kiện không chặn (Non-blocking)
    doorController.update();
    motionTracker.update();
    serialCli.update();
    webServer.update();
    sheetClient.update();
    mqttManager.update();

    // Giám sát nút BOOT vật lý để Factory Reset
    checkFactoryResetButton();
}
