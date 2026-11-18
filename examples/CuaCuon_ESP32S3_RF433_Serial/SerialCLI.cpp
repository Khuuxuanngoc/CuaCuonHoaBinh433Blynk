#include "SerialCLI.h"

SerialCLI* SerialCLI::s_instance = nullptr;

SerialCLI::SerialCLI(RollingDoorController& controller)
    : _controller(controller), _bufIndex(0) {
    s_instance = this;
    memset(_buffer, 0, sizeof(_buffer));
}

SerialCLI::~SerialCLI() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void SerialCLI::begin() {
    // Đăng ký nhận sự kiện từ Controller
    _controller.setOnLearnCallback(staticOnLearn);
    _controller.setOnActionCallback(staticOnAction);
    _controller.setOnSniffCallback(staticOnSniff);
    _controller.setOnStateChangeCallback(staticOnStateChange);

    printHelp();
    printTable();
}

void SerialCLI::update() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        // Xử lý ký tự kết thúc dòng (Enter)
        if (c == '\n' || c == '\r') {
            if (_bufIndex > 0) {
                _buffer[_bufIndex] = '\0';
                processLine(_buffer);
                _bufIndex = 0;
                memset(_buffer, 0, sizeof(_buffer));
            }
        }
        // Xử lý phím xóa lùi (Backspace / Delete)
        else if (c == '\b' || c == 127) {
            if (_bufIndex > 0) {
                _bufIndex--;
                _buffer[_bufIndex] = '\0';
            }
        }
        // Nhận ký tự thông thường
        else {
            if (_bufIndex < BUFFER_SIZE - 1) {
                _buffer[_bufIndex++] = c;
            }
        }
    }
}

void SerialCLI::processLine(char* line) {
    // Xóa khoảng trắng đầu chuỗi
    while (*line == ' ') line++;
    if (*line == '\0') return;

    // Tách lệnh chính và tham số
    char* cmd = strtok(line, " ");
    char* args = strtok(nullptr, ""); // Lấy phần còn lại của chuỗi

    if (args) {
        while (*args == ' ') args++;
    }

    if (strcasecmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0 || strcasecmp(cmd, "menu") == 0) {
        printHelp();
    }
    else if (strcasecmp(cmd, "show") == 0 || strcasecmp(cmd, "list") == 0 || strcasecmp(cmd, "status") == 0) {
        printTable();
    }
    else if (strcasecmp(cmd, "learn") == 0 || strcasecmp(cmd, "hoc") == 0) {
        handleLearn(args);
    }
    else if (strcasecmp(cmd, "cancel") == 0 || strcasecmp(cmd, "huy") == 0) {
        if (_controller.isLearning()) {
            _controller.cancelLearning();
            Serial.println(F("[CLI] Đã hủy chế độ học lệnh."));
        } else {
            Serial.println(F("[CLI] Hiện không trong chế độ học lệnh."));
        }
    }
    else if (strcasecmp(cmd, "test") == 0 || strcasecmp(cmd, "send") == 0 || strcasecmp(cmd, "phat") == 0) {
        handleTest(args);
    }
    else if (strcasecmp(cmd, "save") == 0 || strcasecmp(cmd, "luu") == 0) {
        handleSave();
    }
    else if (strcasecmp(cmd, "load") == 0 || strcasecmp(cmd, "nap") == 0) {
        handleLoad();
    }
    else if (strcasecmp(cmd, "clear") == 0 || strcasecmp(cmd, "xoa") == 0) {
        handleClear();
    }
    else if (strcasecmp(cmd, "sniff") == 0 || strcasecmp(cmd, "scan") == 0 || strcasecmp(cmd, "soi") == 0) {
        handleSniff(args);
    }
    else {
        Serial.print(F("[CLI] Lệnh không hợp lệ: '"));
        Serial.print(cmd);
        Serial.println(F("'. Gõ 'help' hoặc '?' để xem danh sách lệnh."));
    }
}

void SerialCLI::handleLearn(char* args) {
    if (!args || strlen(args) == 0) {
        Serial.println(F("[CLI LỖI] Cú pháp: learn <up|down|stop|lock> hoặc learn <1|2|3|4>"));
        return;
    }

    DoorAction action;
    if (!parseAction(args, action)) {
        Serial.print(F("[CLI LỖI] Nút không xác định: '"));
        Serial.print(args);
        Serial.println(F("'. Chọn một trong các nút: up, down, stop, lock (hoặc 1, 2, 3, 4)."));
        return;
    }

    Serial.println();
    Serial.print(F(">>> [CHẾ ĐỘ HỌC LỆNH] Đang chờ tín hiệu cho nút: "));
    Serial.print(getActionName(action));
    Serial.print(F(" - "));
    Serial.println(getActionDescVi(action));
    Serial.println(F(">>> Hướng remote vào anten thu 433MHz và BẤM GIỮ NÚT trong 1-2 giây..."));
    Serial.println(F(">>> (Thời gian chờ: 15 giây. Gõ 'cancel' để hủy bỏ)"));

    _controller.startLearning(action);
}

void SerialCLI::handleTest(char* args) {
    if (!args || strlen(args) == 0) {
        Serial.println(F("[CLI LỖI] Cú pháp: test <up|down|stop|lock> hoặc test <1|2|3|4>"));
        return;
    }

    DoorAction action;
    if (!parseAction(args, action)) {
        Serial.print(F("[CLI LỖI] Nút không xác định: '"));
        Serial.print(args);
        Serial.println(F("'. Chọn: up, down, stop, lock"));
        return;
    }

    if (!_controller.hasValidCommand(action)) {
        Serial.print(F("[CLI CẢNH BÁO] Nút "));
        Serial.print(getActionName(action));
        Serial.println(F(" CHƯA ĐƯỢC HỌC LỆNH! Vui lòng dùng lệnh 'learn' trước."));
        return;
    }

    const RFCommand& cmd = _controller.getCommand(action);
    Serial.print(F(">>> [PHÁT LỆNH] Đang phát tín hiệu RF cho nút: "));
    Serial.print(getActionName(action));
    Serial.print(F(" (Mã: 0x"));
    Serial.print(cmd.code, HEX);
    Serial.println(F(")..."));

    bool ok = _controller.sendCommand(action);
    if (ok) {
        Serial.println(F(">>> [THÀNH CÔNG] Đã phát sóng RF 433MHz xong!"));
    } else {
        Serial.println(F(">>> [LỖI] Không thể phát tín hiệu!"));
    }
}

void SerialCLI::handleSave() {
    Serial.println(F(">>> Đang ghi dữ liệu vào bộ nhớ Flash..."));
    if (_controller.save()) {
        Serial.println(F(">>> [THÀNH CÔNG] Đã lưu 4 lệnh vào Flash! Không bị mất khi khởi động lại."));
    } else {
        Serial.println(F(">>> [LỖI] Ghi Flash thất bại!"));
    }
}

void SerialCLI::handleLoad() {
    Serial.println(F(">>> Đang nạp dữ liệu từ Flash..."));
    if (_controller.load()) {
        Serial.println(F(">>> [THÀNH CÔNG] Đã nạp lại dữ liệu từ Flash!"));
        printTable();
    } else {
        Serial.println(F(">>> [LƯU Ý] Chưa có dữ liệu hợp lệ trong Flash!"));
    }
}

void SerialCLI::handleClear() {
    Serial.println(F(">>> Đang xóa dữ liệu trong Flash và RAM..."));
    if (_controller.clear()) {
        Serial.println(F(">>> [THÀNH CÔNG] Đã xóa toàn bộ lệnh!"));
        printTable();
    } else {
        Serial.println(F(">>> [LỖI] Xóa dữ liệu thất bại!"));
    }
}

void SerialCLI::handleSniff(char* args) {
    if (!args || strlen(args) == 0) {
        bool current = _controller.isSnifferMode();
        Serial.print(F("[SNIFFER] Trạng thái hiện tại: "));
        Serial.println(current ? F("ĐANG BẬT (ON)") : F("ĐANG TẮT (OFF)"));
        Serial.println(F("[SNIFFER] Cú pháp thay đổi: sniff on | sniff off"));
        return;
    }

    if (strcasecmp(args, "on") == 0 || strcmp(args, "1") == 0) {
        _controller.setSnifferMode(true);
        Serial.println(F("[SNIFFER] Đã BẬT chế độ bắt gói tin RF 433 liên tục. Bấm remote bất kỳ để xem mã."));
    } else if (strcasecmp(args, "off") == 0 || strcmp(args, "0") == 0) {
        _controller.setSnifferMode(false);
        Serial.println(F("[SNIFFER] Đã TẮT chế độ bắt gói tin."));
    } else {
        Serial.println(F("[CLI LỖI] Cú pháp: sniff on hoặc sniff off"));
    }
}

void SerialCLI::printTable() {
    Serial.println();
    Serial.println(F("+-----+--------+-------------------+------------+------------+------+-------+-----------+"));
    Serial.println(F("| STT | NÚT    | CHỨC NĂNG         | MÃ HEX     | MÃ DEC     | BITS | PROTO | XUNG (us) |"));
    Serial.println(F("+-----+--------+-------------------+------------+------------+------+-------+-----------+"));

    for (uint8_t i = 0; i < ACTION_COUNT; i++) {
        DoorAction act = (DoorAction)i;
        const RFCommand& cmd = _controller.getCommand(act);

        char row[120];
        if (cmd.isValid && cmd.code != 0) {
            snprintf(row, sizeof(row), "|  %d  | %-6s | %-17s | 0x%08X | %-10lu |  %2u  |   %u   |   %-7u |",
                     i + 1,
                     getActionName(act),
                     (i == 0) ? "Len (Mo cua)" : (i == 1) ? "Xuong (Dong cua)" : (i == 2) ? "Dung (Tam dung)" : "Khoa (Khoa cua)",
                     (unsigned int)cmd.code,
                     (unsigned long)cmd.code,
                     cmd.bitLength,
                     cmd.protocol,
                     cmd.pulseLength);
        } else {
            snprintf(row, sizeof(row), "|  %d  | %-6s | %-17s | [CHUA HOC] | ---------- |  --  |   -   |   -----   |",
                     i + 1,
                     getActionName(act),
                     (i == 0) ? "Len (Mo cua)" : (i == 1) ? "Xuong (Dong cua)" : (i == 2) ? "Dung (Tam dung)" : "Khoa (Khoa cua)");
        }
        Serial.println(row);
    }
    Serial.println(F("+-----+--------+-------------------+------------+------------+------+-------+-----------+"));

    Serial.print(F("Trạng thái bộ nhớ Flash: "));
    if (_controller.isModified()) {
        Serial.println(F("[CÓ THAY ĐỔI CHƯA LƯU! Gõ 'save' để ghi vào Flash]"));
    } else {
        Serial.println(F("[ĐÃ ĐỒNG BỘ VỚI FLASH]"));
    }
    Serial.println();
}

void SerialCLI::printHelp() {
    Serial.println();
    Serial.println(F("=========================================================================="));
    Serial.println(F("           DANH SÁCH CÂU LỆNH ĐIỀU KHIỂN CỬA CUỐN RF 433MHz               "));
    Serial.println(F("=========================================================================="));
    Serial.println(F(" 1. learn <nut>    : Học lệnh RF cho nút (up, down, stop, lock hoặc 1..4)"));
    Serial.println(F("    Ví dụ: learn up   (hoặc: learn 1)"));
    Serial.println(F(" 2. cancel         : Hủy chế độ đang học lệnh nếu không muốn đợi"));
    Serial.println(F(" 3. show           : Hiển thị bảng mã lệnh 4 trạng thái hiện tại"));
    Serial.println(F(" 4. test <nut>     : Thử phát tín hiệu RF điều khiển cửa cuốn"));
    Serial.println(F("    Ví dụ: test up   (hoặc: test 1)"));
    Serial.println(F(" 5. save           : Lưu vĩnh viễn các mã đã học vào Flash (NVS / EEPROM)"));
    Serial.println(F(" 6. load           : Nạp lại mã từ bộ nhớ Flash"));
    Serial.println(F(" 7. clear          : Xóa trắng toàn bộ mã lệnh trong RAM và Flash"));
    Serial.println(F(" 8. sniff <on/off> : Chế độ 'thám thính' - quét mọi sóng RF 433 trong không khí"));
    Serial.println(F(" 9. help / ?       : Hiển thị lại menu trợ giúp này"));
    Serial.println(F("=========================================================================="));
}

void SerialCLI::staticOnLearn(DoorAction action, const RFCommand& cmd, bool success) {
    if (success) {
        Serial.println();
        Serial.println(F("--------------------------------------------------"));
        Serial.print(F(">>> [HỌC THÀNH CÔNG] Nút: "));
        Serial.print(getActionName(action));
        Serial.print(F(" ("));
        Serial.print(getActionDescVi(action));
        Serial.println(F(")"));
        Serial.print(F("    - Mã Hex     : 0x"));
        Serial.println(cmd.code, HEX);
        Serial.print(F("    - Mã Dec     : "));
        Serial.println(cmd.code);
        Serial.print(F("    - Số Bit     : "));
        Serial.println(cmd.bitLength);
        Serial.print(F("    - Protocol   : "));
        Serial.println(cmd.protocol);
        Serial.print(F("    - Pulse delay: "));
        Serial.print(cmd.pulseLength);
        Serial.println(F(" us"));
        Serial.println(F(">>> Gõ 'test <nut>' để thử phát hoặc 'save' để lưu vào Flash!"));
        Serial.println(F("--------------------------------------------------"));
    } else {
        Serial.println();
        Serial.print(F(">>> [HỌC THẤT BẠI] Hết thời gian chờ (Timeout) cho nút: "));
        Serial.println(getActionName(action));
        Serial.println(F(">>> Vui lòng thử lại với lệnh 'learn' và giữ nút remote gần anten hơn."));
    }
}

void SerialCLI::staticOnAction(DoorAction action, const RFCommand& cmd) {
    // Thông báo khi lệnh được phát xong
    (void)action;
    (void)cmd;
}

void SerialCLI::staticOnSniff(const RFCommand& cmd) {
    Serial.print(F("[SNIFF RF433] Mã: 0x"));
    Serial.print(cmd.code, HEX);
    Serial.print(F(" ("));
    Serial.print(cmd.code);
    Serial.print(F(") | Bit: "));
    Serial.print(cmd.bitLength);
    Serial.print(F(" | Proto: "));
    Serial.print(cmd.protocol);
    Serial.print(F(" | Pulse: "));
    Serial.print(cmd.pulseLength);
    Serial.println(F(" us"));
}

void SerialCLI::staticOnStateChange(ControllerState state, const char* message) {
    (void)state;
    (void)message;
}
