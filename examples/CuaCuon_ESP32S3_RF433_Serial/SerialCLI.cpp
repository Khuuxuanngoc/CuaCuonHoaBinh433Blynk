#include "SerialCLI.h"
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

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

        if (c == '\n' || c == '\r') {
            if (_bufIndex > 0) {
                _buffer[_bufIndex] = '\0';
                processLine(_buffer);
                _bufIndex = 0;
                memset(_buffer, 0, sizeof(_buffer));
            }
        }
        else if (c == '\b' || c == 127) {
            if (_bufIndex > 0) {
                _bufIndex--;
                _buffer[_bufIndex] = '\0';
            }
        }
        else {
            if (_bufIndex < BUFFER_SIZE - 1) {
                _buffer[_bufIndex++] = c;
            }
        }
    }
}

void SerialCLI::processLine(char* line) {
    while (*line == ' ') line++;
    if (*line == '\0') return;

    char* cmd = strtok(line, " ");
    char* args = strtok(nullptr, "");

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
            Serial.println(F("[CLI] Da huy che do hoc lenh."));
        } else {
            Serial.println(F("[CLI] Hien khong trong che do hoc lenh."));
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
    else if (strcasecmp(cmd, "export") == 0 || strcasecmp(cmd, "xuat") == 0) {
        handleExport();
    }
    else if (strcasecmp(cmd, "import") == 0 || strcasecmp(cmd, "nhap") == 0) {
        handleImport(args);
    }
    else if (strcasecmp(cmd, "set") == 0 || strcasecmp(cmd, "gan") == 0) {
        handleSet(args);
    }
    else if (strcasecmp(cmd, "reset") == 0 || strcasecmp(cmd, "factory-reset") == 0) {
        handleReset();
    }
    else if (strcasecmp(cmd, "wifi") == 0 || strcasecmp(cmd, "ip") == 0) {
        handleWifiInfo();
    }
    else {
        Serial.print(F("[CLI] Lenh khong hop le: '"));
        Serial.print(cmd);
        Serial.println(F("'. Go 'help' hoac '?' de xem danh sach lenh."));
    }
}

void SerialCLI::handleLearn(char* args) {
    if (!args || strlen(args) == 0) {
        Serial.println(F("[CLI LOI] Cu phap: learn <up|down|stop|lock> hoac learn <1|2|3|4>"));
        return;
    }

    DoorAction action;
    if (!parseAction(args, action)) {
        Serial.print(F("[CLI LOI] Nut khong xac dinh: '"));
        Serial.print(args);
        Serial.println(F("'. Chon mot trong cac nut: up, down, stop, lock (hoac 1, 2, 3, 4)."));
        return;
    }

    Serial.println();
    Serial.print(F(">>> [CHE DO HOC LENH] Dang cho tin hieu cho nut: "));
    Serial.print(getActionName(action));
    Serial.print(F(" - "));
    Serial.println(getActionDescNoAccent(action));
    Serial.println(F(">>> Huong remote vao anten thu 433MHz va BAM GIU NUT trong 1-2 giay..."));
    Serial.println(F(">>> (Thoi gian cho: 15 giay. Go 'cancel' de huy bo)"));

    _controller.startLearning(action);
}

void SerialCLI::handleTest(char* args) {
    if (!args || strlen(args) == 0) {
        Serial.println(F("[CLI LOI] Cu phap: test <up|down|stop|lock> hoac test <1|2|3|4>"));
        return;
    }

    DoorAction action;
    if (!parseAction(args, action)) {
        Serial.print(F("[CLI LOI] Nut khong xac dinh: '"));
        Serial.print(args);
        Serial.println(F("'. Chon mot trong cac nut: up, down, stop, lock."));
        return;
    }

    Serial.print(F(">>> [PHAT LENH] Dang phat tin hieu cho nut: "));
    Serial.print(getActionName(action));
    Serial.print(F(" ("));
    Serial.print(getActionDescNoAccent(action));
    Serial.println(F(")..."));

    bool ok = _controller.sendCommand(action);
    if (ok) {
        Serial.println(F(">>> [THANH CONG] Da phat 10 chu ky song RF 433MHz!"));
    } else {
        Serial.println(F(">>> [THAT BAI] Nut nay chua duoc hoc lenh! Hay dung lenh 'learn' truoc."));
    }
}

void SerialCLI::handleSave() {
    Serial.print(F(">>> [LUU BO NHO] Dang ghi du lieu vao Flash ("));
    Serial.print(F(BOARD_TARGET_NAME));
    Serial.println(F(")..."));

    if (_controller.save()) {
        Serial.println(F(">>> [THANH CONG] Da luu vinh vien toan bo ma lenh vao bo nho Flash."));
    } else {
        Serial.println(F(">>> [THAT BAI] Khong the ghi du lieu vao Flash!"));
    }
}

void SerialCLI::handleLoad() {
    Serial.println(F(">>> [TAI BO NHO] Dang doc lai du lieu tu Flash..."));
    if (_controller.load()) {
        Serial.println(F(">>> [THANH CONG] Da tai toan bo ma lenh tu Flash vao RAM."));
        printTable();
    } else {
        Serial.println(F(">>> [THONG BAO] Chua co du lieu ma lenh nao duoc luu trong Flash."));
    }
}

void SerialCLI::handleClear() {
    Serial.println(F(">>> [XOA DU LIEU] Dang xoa toan bo ma lenh RF..."));
    if (_controller.clear()) {
        Serial.println(F(">>> [THANH CONG] Da xoa trang toan bo ma lenh trong RAM va Flash."));
        printTable();
    } else {
        Serial.println(F(">>> [THAT BAI] Loi khi xoa bo nho Flash!"));
    }
}

void SerialCLI::handleSniff(char* args) {
    if (!args || strlen(args) == 0) {
        bool current = _controller.isSnifferMode();
        Serial.print(F("[CLI] Che do Sniffer hien tai: "));
        Serial.println(current ? F("BAT (ON)") : F("TAT (OFF)"));
        Serial.println(F("Cu phap: sniff on  hoac  sniff off"));
        return;
    }

    if (strcasecmp(args, "on") == 0 || strcmp(args, "1") == 0) {
        _controller.setSnifferMode(true);
        Serial.println(F(">>> [SNIFFER ON] Bat che do lang nghe moi song RF 433MHz ngoai khong khi..."));
        Serial.println(F(">>> Bam bat ky nut remote nao de xem goi tin giai ma."));
    } else if (strcasecmp(args, "off") == 0 || strcmp(args, "0") == 0) {
        _controller.setSnifferMode(false);
        Serial.println(F(">>> [SNIFFER OFF] Da tat che do Sniffer."));
    } else {
        Serial.println(F("[CLI LOI] Cu phap: sniff on  hoac  sniff off"));
    }
}

void SerialCLI::handleExport() {
    Serial.println();
    Serial.println(F("=========================================================="));
    Serial.println(F("        DU LIEU XUAT MA LENH (SAO CHEP CHO ESP8266)       "));
    Serial.println(F("=========================================================="));

    const RFCommand& cUp   = _controller.getCommand(ACTION_UP);
    const RFCommand& cDown = _controller.getCommand(ACTION_DOWN);
    const RFCommand& cStop = _controller.getCommand(ACTION_STOP);
    const RFCommand& cLock = _controller.getCommand(ACTION_LOCK);

    Serial.println(F("--- [CACH 1: Lenh Import truc tiep tren Serial ESP8266] ---"));
    Serial.print(F("import 0x"));
    Serial.print(cUp.code, HEX);
    Serial.print(F(" 0x"));
    Serial.print(cDown.code, HEX);
    Serial.print(F(" 0x"));
    Serial.print(cStop.code, HEX);
    Serial.print(F(" 0x"));
    Serial.print(cLock.code, HEX);
    Serial.print(F(" "));
    Serial.print(cUp.bitLength);
    Serial.print(F(" "));
    Serial.print(cUp.protocol);
    Serial.print(F(" "));
    Serial.println(cUp.pulseLength);

    Serial.println();
    Serial.println(F("--- [CACH 2: Dan vao Config.h cua ESP8266] ---"));
    Serial.print(F("#define PRESET_CODE_UP     0x")); Serial.println(cUp.code, HEX);
    Serial.print(F("#define PRESET_CODE_DOWN   0x")); Serial.println(cDown.code, HEX);
    Serial.print(F("#define PRESET_CODE_STOP   0x")); Serial.println(cStop.code, HEX);
    Serial.print(F("#define PRESET_CODE_LOCK   0x")); Serial.println(cLock.code, HEX);
    Serial.print(F("#define PRESET_BIT_LENGTH  ")); Serial.println(cUp.bitLength);
    Serial.print(F("#define PRESET_PROTOCOL    ")); Serial.println(cUp.protocol);
    Serial.print(F("#define PRESET_PULSE_LENGTH ")); Serial.println(cUp.pulseLength);
    Serial.println(F("=========================================================="));
    Serial.println();
}

void SerialCLI::handleImport(char* args) {
    if (!args || strlen(args) == 0) {
        Serial.println(F("[CLI LOI] Cu phap: import <code_up> <code_down> <code_stop> <code_lock> [bits] [proto] [pulse]"));
        return;
    }

    char* token = strtok(args, " ");
    if (!token) return;
    uint32_t cUp = strtoul(token, nullptr, 0);

    token = strtok(nullptr, " ");
    if (!token) return;
    uint32_t cDown = strtoul(token, nullptr, 0);

    token = strtok(nullptr, " ");
    if (!token) return;
    uint32_t cStop = strtoul(token, nullptr, 0);

    token = strtok(nullptr, " ");
    if (!token) return;
    uint32_t cLock = strtoul(token, nullptr, 0);

    uint8_t bits = 24;
    uint8_t proto = 1;
    uint16_t pulse = 350;

    token = strtok(nullptr, " ");
    if (token) bits = (uint8_t)atoi(token);
    token = strtok(nullptr, " ");
    if (token) proto = (uint8_t)atoi(token);
    token = strtok(nullptr, " ");
    if (token) pulse = (uint16_t)atoi(token);

    _controller.setCommand(ACTION_UP, RFCommand(cUp, bits, proto, pulse));
    _controller.setCommand(ACTION_DOWN, RFCommand(cDown, bits, proto, pulse));
    _controller.setCommand(ACTION_STOP, RFCommand(cStop, bits, proto, pulse));
    _controller.setCommand(ACTION_LOCK, RFCommand(cLock, bits, proto, pulse));

    Serial.println(F(">>> [NHAP THANH CONG] Da nhan du 4 ma lenh tu chuoi!"));
    if (_controller.save()) {
        Serial.println(F(">>> [THANH CONG] Da tu dong luu vinh vien vao bo nho Flash."));
    }
    printTable();
}

void SerialCLI::handleSet(char* args) {
    if (!args || strlen(args) == 0) {
        Serial.println(F("[CLI LOI] Cu phap:"));
        Serial.println(F("  set <up|down|stop|lock> <code> [bits] [proto] [pulse]"));
        Serial.println(F("  set pass <mat_khau_master_moi>"));
        Serial.println(F("  set ap <mat_khau_wifi_ap_moi>"));
        Serial.println(F("  set wifi <ssid> <password>"));
        Serial.println(F("  set gas <webhook_url>"));
        return;
    }

    char* subCmd = strtok(args, " ");
    if (!subCmd) return;

    // Đổi Master Password
    if (strcasecmp(subCmd, "pass") == 0) {
        char* newPass = strtok(nullptr, " ");
        if (!newPass || strlen(newPass) == 0) {
            Serial.println(F("[CLI LOI] Thieu mat khau moi!"));
            return;
        }
        SystemConfig cfg;
        _controller.getStorage().loadSystemConfig(cfg);
        strncpy(cfg.webPassword, newPass, sizeof(cfg.webPassword) - 1);
        _controller.getStorage().saveSystemConfig(cfg);
        Serial.print(F(">>> [THANH CONG] Da doi Master Password thanh: "));
        Serial.println(cfg.webPassword);
        return;
    }

    // Đổi WiFi AP Password
    if (strcasecmp(subCmd, "ap") == 0) {
        char* newAp = strtok(nullptr, " ");
        if (!newAp || strlen(newAp) < 8) {
            Serial.println(F("[CLI LOI] Mat khau WiFi AP phai co it nhat 8 ky tu!"));
            return;
        }
        SystemConfig cfg;
        _controller.getStorage().loadSystemConfig(cfg);
        strncpy(cfg.apPassword, newAp, sizeof(cfg.apPassword) - 1);
        _controller.getStorage().saveSystemConfig(cfg);
        WiFi.softAP(WIFI_AP_SSID, cfg.apPassword);
        Serial.print(F(">>> [THANH CONG] Da doi mat khau WiFi AP thanh: "));
        Serial.println(cfg.apPassword);
        return;
    }

    // Đổi WiFi Station
    if (strcasecmp(subCmd, "wifi") == 0) {
        char* ssid = strtok(nullptr, " ");
        char* pass = strtok(nullptr, " ");
        if (!ssid) {
            Serial.println(F("[CLI LOI] Cu phap: set wifi <ssid> <password>"));
            return;
        }
        SystemConfig cfg;
        _controller.getStorage().loadSystemConfig(cfg);
        strncpy(cfg.wifiSSID, ssid, sizeof(cfg.wifiSSID) - 1);
        strncpy(cfg.wifiPass, pass ? pass : "", sizeof(cfg.wifiPass) - 1);
        _controller.getStorage().saveSystemConfig(cfg);
        WiFi.begin(cfg.wifiSSID, cfg.wifiPass);
        Serial.print(F(">>> [THANH CONG] Da luu WiFi gia dinh: "));
        Serial.println(cfg.wifiSSID);
        return;
    }

    // Đổi URL Google Apps Script
    if (strcasecmp(subCmd, "gas") == 0) {
        char* url = strtok(nullptr, " ");
        if (!url) {
            Serial.println(F("[CLI LOI] Cu phap: set gas <url>"));
            return;
        }
        SystemConfig cfg;
        _controller.getStorage().loadSystemConfig(cfg);
        strncpy(cfg.gasWebhookUrl, url, sizeof(cfg.gasWebhookUrl) - 1);
        _controller.getStorage().saveSystemConfig(cfg);
        Serial.println(F(">>> [THANH CONG] Da luu Google Apps Script Webhook URL moi!"));
        return;
    }

    // Gán mã lệnh nút RF
    DoorAction action;
    if (!parseAction(subCmd, action)) {
        Serial.println(F("[CLI LOI] Lenh hoac nut khong hop le"));
        return;
    }

    char* codeStr = strtok(nullptr, " ");
    if (!codeStr) {
        Serial.println(F("[CLI LOI] Thieu gia tri ma code"));
        return;
    }
    uint32_t code = strtoul(codeStr, nullptr, 0);

    const RFCommand& existing = _controller.getCommand(action);
    uint8_t bits = existing.isValid ? existing.bitLength : 24;
    uint8_t proto = existing.isValid ? existing.protocol : 1;
    uint16_t pulse = existing.isValid ? existing.pulseLength : 350;

    char* token = strtok(nullptr, " ");
    if (token) bits = (uint8_t)atoi(token);
    token = strtok(nullptr, " ");
    if (token) proto = (uint8_t)atoi(token);
    token = strtok(nullptr, " ");
    if (token) pulse = (uint16_t)atoi(token);

    _controller.setCommand(action, RFCommand(code, bits, proto, pulse));
    Serial.print(F(">>> Da thiet lap thanh cong ma nut "));
    Serial.println(getActionName(action));
    Serial.println(F(">>> Go 'save' de ghi vao Flash hoac 'test <nut>' de phat thu."));
}

void SerialCLI::handleReset() {
    Serial.println(F(">>> [KHOI PHUC CAI DAT GOC] Dang xoa toan bo Flash & cau hinh..."));
    _controller.getStorage().resetToFactoryDefaults();
    _controller.clear();
    Serial.println(F(">>> [THANH CONG] Da khoi phuc cai dat goc toan bo he thong!"));
    Serial.println(F(">>> ESP se tu dong khoi dong lai sau 2 giay..."));
    delay(2000);
    ESP.restart();
}

void SerialCLI::handleWifiInfo() {
    SystemConfig cfg;
    _controller.getStorage().loadSystemConfig(cfg);

    Serial.println();
    Serial.println(F("=========================================================="));
    Serial.println(F("             THONG TIN MANG & BAO MAT HE THONG            "));
    Serial.println(F("=========================================================="));
    Serial.print(F(" - WiFi AP phat ra : ")); Serial.print(F(WIFI_AP_SSID));
    Serial.print(F(" | Pass: ")); Serial.println(cfg.apPassword);
    Serial.print(F(" - Dia chi IP AP   : http://")); Serial.println(WiFi.softAPIP());
    Serial.print(F(" - WiFi nha (STA)  : ")); Serial.print(cfg.wifiSSID);
    Serial.print(F(" | Trang thai: "));
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("DA KET NOI (IP: "));
        Serial.print(WiFi.localIP());
        Serial.println(F(")"));
    } else {
        Serial.println(F("Chua ket noi"));
    }
    Serial.print(F(" - Master Password : ")); Serial.println(cfg.webPassword);
    Serial.print(F(" - GAS Webhook URL : ")); Serial.println(strlen(cfg.gasWebhookUrl) > 0 ? cfg.gasWebhookUrl : "(Chua cau hinh)");
    Serial.println(F("=========================================================="));
    Serial.println();
}

void SerialCLI::printTable() {
    Serial.println();
    Serial.println(F("+-----+--------+-------------------+------------+------------+------+-------+-----------+"));
    Serial.println(F("| STT | NUT    | CHUC NANG         | MA HEX     | MA DEC     | BITS | PROTO | XUNG (us) |"));
    Serial.println(F("+-----+--------+-------------------+------------+------------+------+-------+-----------+"));

    for (uint8_t i = 0; i < ACTION_COUNT; i++) {
        DoorAction act = (DoorAction)i;
        const RFCommand& cmd = _controller.getCommand(act);

        char row[100];
        char hexStr[12];
        char decStr[12];

        if (cmd.isValid) {
            snprintf(hexStr, sizeof(hexStr), "0x%06lX", (unsigned long)cmd.code);
            snprintf(decStr, sizeof(decStr), "%-10lu", (unsigned long)cmd.code);
            snprintf(row, sizeof(row), "|  %u  | %-6s | %-17s | %-10s | %-10s |  %2u  |   %u   |   %4u    |",
                     i + 1,
                     getActionName(act),
                     getActionDescNoAccent(act),
                     hexStr,
                     decStr,
                     cmd.bitLength,
                     cmd.protocol,
                     cmd.pulseLength);
        } else {
            snprintf(row, sizeof(row), "|  %u  | %-6s | %-17s | [CHUA HOC] | ---------- |  --  |   -   |   ----    |",
                     i + 1,
                     getActionName(act),
                     getActionDescNoAccent(act));
        }
        Serial.println(row);
    }
    Serial.println(F("+-----+--------+-------------------+------------+------------+------+-------+-----------+"));

    Serial.print(F("Trang thai bo nho Flash: "));
    if (_controller.isModified()) {
        Serial.println(F("[CO THAY DOI CHUA LUU! Go 'save' de ghi vao Flash]"));
    } else {
        Serial.println(F("[DA DONG BO VOI FLASH]"));
    }
    Serial.println();
}

void SerialCLI::printHelp() {
    Serial.println();
    Serial.println(F("=========================================================================="));
    Serial.println(F("           DANH SACH CAU LENH DIEU KHIEN CUA CUON RF 433MHz               "));
    Serial.println(F("=========================================================================="));
    Serial.println(F(" 1. learn <nut>       : Hoc lenh RF (up, down, stop, lock hoac 1..4)"));
    Serial.println(F(" 2. test <nut>        : Thu phat tin hieu RF dieu khien cua cuon"));
    Serial.println(F(" 3. show              : Xem bang ma 4 trang thai RF"));
    Serial.println(F(" 4. save / load       : Luu / Tai lai ma lenh tu bo nho Flash"));
    Serial.println(F(" 5. export / import   : Xuat / Nhap ma chuyen tiep giua ESP32-S3 & ESP8266"));
    Serial.println(F(" 6. set pass <pwd>    : Doi Master Password nhanh qua cong Serial"));
    Serial.println(F(" 7. set ap <pwd>      : Doi mat khau WiFi AP cua ESP (toi thieu 8 ky tu)"));
    Serial.println(F(" 8. set wifi <s> <p>  : Cau hinh WiFi gia dinh"));
    Serial.println(F(" 9. wifi / ip         : Xem trang thai WiFi va IP hien tai"));
    Serial.println(F("10. reset             : Khoi phuc toan bo cai dat goc (Factory Reset)"));
    Serial.println(F("11. sniff <on/off>    : Bat/tat quet moi song RF 433 bat duoc"));
    Serial.println(F("12. help / ?          : Xem danh muc menu nay"));
    Serial.println(F("=========================================================================="));
}

void SerialCLI::staticOnLearn(DoorAction action, const RFCommand& cmd, bool success) {
    if (success) {
        Serial.println();
        Serial.println(F("--------------------------------------------------"));
        Serial.print(F(">>> [HOC THANH CONG] Nut: "));
        Serial.print(getActionName(action));
        Serial.print(F(" ("));
        Serial.print(getActionDescNoAccent(action));
        Serial.println(F(")"));
        Serial.print(F("    - Ma Hex     : 0x"));
        Serial.println(cmd.code, HEX);
        Serial.print(F("    - Ma Dec     : "));
        Serial.println(cmd.code);
        Serial.print(F("    - So Bit     : "));
        Serial.println(cmd.bitLength);
        Serial.print(F("    - Protocol   : "));
        Serial.println(cmd.protocol);
        Serial.print(F("    - Pulse delay: "));
        Serial.print(cmd.pulseLength);
        Serial.println(F(" us"));
        Serial.println(F(">>> Go 'test <nut>' de thu phat hoac 'save' de luu vao Flash!"));
        Serial.println(F("--------------------------------------------------"));
    } else {
        Serial.println();
        Serial.print(F(">>> [HOC THAT BAI] Het thoi gian cho (Timeout) cho nut: "));
        Serial.println(getActionName(action));
        Serial.println(F(">>> Vui long thu lai voi lenh 'learn' va giu nut remote gan anten hon."));
    }
}

void SerialCLI::staticOnAction(DoorAction action, const RFCommand& cmd) {
    (void)action;
    (void)cmd;
}

void SerialCLI::staticOnSniff(const RFCommand& cmd) {
    Serial.print(F("[SNIFF RF433] Ma: 0x"));
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
