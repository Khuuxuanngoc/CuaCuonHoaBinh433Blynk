/**
 * ============================================================================
 * Dự án   : Điều Khiển Cửa Cuốn RF 433MHz - Phiên Bản 1.0 (Serial CLI)
 * Nền tảng: ESP32-S3 (Sẵn sàng Port sang ESP8266)
 * Tác giả : Hệ thống Robotics & IoT Hòa Bình
 * Chuẩn   : Tuân thủ AI_WORKSPACE.md & Kiến trúc Hướng Đối Tượng (OOP)
 * ============================================================================
 */

#include "Config.h"
#include "RollingDoorController.h"
#include "SerialCLI.h"

// Khởi tạo các đối tượng điều khiển theo kiến trúc OOP
RollingDoorController doorController;
SerialCLI serialCli(doorController);

void setup() {
    // Đảm bảo chân phát RF ở mức LOW ngay khi khởi động (chống treo sóng do trở pull-up của FET)
    pinMode(RF_TX_PIN, OUTPUT);
    digitalWrite(RF_TX_PIN, LOW);

    // Khởi tạo giao tiếp Serial Monitor
    Serial.begin(SERIAL_BAUD_RATE);

    // Chờ cổng Serial khả dụng (đặc biệt cần thiết cho native USB trên ESP32-S3)
    uint32_t startWait = millis();
    while (!Serial && (millis() - startWait < 2000)) {
        delay(10);
    }

    Serial.println();
    Serial.println(F("=========================================================="));
    Serial.println(F("   HỆ THỐNG ĐIỀU KHIỂN CỬA CUỐN RF 433MHz - PHIÊN BẢN V1  "));
    Serial.print(F("   Nền tảng vi điều khiển: "));
    Serial.println(F(BOARD_TARGET_NAME));
    Serial.println(F("=========================================================="));

    // Khởi tạo bộ điều khiển trung tâm với cấu hình chân từ Config.h
    if (!doorController.begin(RF_RX_PIN, RF_TX_PIN, STATUS_LED_PIN)) {
        Serial.println(F("[LỖI KHỞI TẠO] Không thể khởi động RollingDoorController!"));
    }

    // Khởi tạo giao diện dòng lệnh Serial CLI
    serialCli.begin();
}

void loop() {
    // Cập nhật State Machine và xử lý sự kiện không chặn (Non-blocking)
    doorController.update();

    // Đọc và xử lý lệnh từ Serial Monitor
    serialCli.update();
}
