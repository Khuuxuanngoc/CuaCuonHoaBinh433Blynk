#ifndef SERIAL_CLI_H
#define SERIAL_CLI_H

#include <Arduino.h>
#include "RollingDoorController.h"

/**
 * @class SerialCLI
 * @brief Giao diện dòng lệnh qua cổng Serial (Serial Monitor)
 * Xử lý dữ liệu không chặn (Non-blocking), bộ đệm tĩnh cố định (Zero-malloc).
 */
class SerialCLI {
public:
    explicit SerialCLI(RollingDoorController& controller);
    ~SerialCLI();

    /**
     * @brief Khởi tạo giao diện Serial CLI và đăng ký Callback
     */
    void begin();

    /**
     * @brief Vòng lặp đọc và xử lý lệnh từ Serial (Gọi trong loop())
     */
    void update();

    /**
     * @brief In bảng danh sách lệnh hiện tại ra Serial
     */
    void printTable();

    /**
     * @brief In danh mục menu trợ giúp các câu lệnh
     */
    void printHelp();

private:
    void processLine(char* line);
    void handleLearn(char* args);
    void handleTest(char* args);
    void handleSave();
    void handleLoad();
    void handleClear();
    void handleSniff(char* args);

    // Callbacks tĩnh nhận sự kiện từ controller
    static void staticOnLearn(DoorAction action, const RFCommand& cmd, bool success);
    static void staticOnAction(DoorAction action, const RFCommand& cmd);
    static void staticOnSniff(const RFCommand& cmd);
    static void staticOnStateChange(ControllerState state, const char* message);

    RollingDoorController& _controller;

    static const uint8_t BUFFER_SIZE = 64;
    char _buffer[BUFFER_SIZE];
    uint8_t _bufIndex;

    static SerialCLI* s_instance;
};

#endif // SERIAL_CLI_H
