#include "GoogleSheetClient.h"
#include "RollingDoorController.h"

GoogleSheetClient::GoogleSheetClient()
    : _controller(nullptr),
      _lastAttemptTime(0),
      _lastPingTime(0),
      _bootNoticeSent(false) {
    memset(_webhookUrl, 0, sizeof(_webhookUrl));
    memset(_token, 0, sizeof(_token));
    memset(&_pending, 0, sizeof(_pending));
}

GoogleSheetClient::~GoogleSheetClient() {
}

bool GoogleSheetClient::begin(const char* webhookUrl, const char* token) {
    if (webhookUrl && strlen(webhookUrl) > 0) {
        strncpy(_webhookUrl, webhookUrl, sizeof(_webhookUrl) - 1);
    }
    if (token && strlen(token) > 0) {
        strncpy(_token, token, sizeof(_token) - 1);
    }
    return true;
}

void GoogleSheetClient::update() {
    if (!isConfigured() || WiFi.status() != WL_CONNECTED) {
        return;
    }

    uint32_t now = millis();

    // 1. Gửi thông báo Boot khi vừa có điện và kết nối WiFi lần đầu tiên
    if (!_bootNoticeSent) {
        sendBootNotice("Power-on Reset / Co dien lai");
        _bootNoticeSent = true;
        _lastPingTime = now;
        return;
    }

    // 2. Gửi sự kiện đang chờ (nếu có)
    if (_pending.isPending && (now - _lastAttemptTime >= 5000)) {
        _lastAttemptTime = now;
        executePost();
        return;
    }

    // 3. Gửi Heartbeat định kỳ mỗi 60 giây (đồng bộ trạng thái Online và nhận lệnh chờ)
    if (now - _lastPingTime >= 60000) {
        _lastPingTime = now;
        sendHeartbeat();
    }
}

void GoogleSheetClient::sendEvent(const char* event, const char* doorState, uint32_t openDurationSec, const char* source) {
    if (!isConfigured()) return;

    strncpy(_pending.event, event, sizeof(_pending.event) - 1);
    strncpy(_pending.doorState, doorState, sizeof(_pending.doorState) - 1);
    _pending.durationSec = openDurationSec;

    if (source) {
        strncpy(_pending.source, source, sizeof(_pending.source) - 1);
    } else {
        strncpy(_pending.source, "Remote RF", sizeof(_pending.source) - 1);
    }

    _pending.isPending = true;
}

void GoogleSheetClient::sendBootNotice(const char* reason) {
    if (!isConfigured() || WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(8000);

    if (http.begin(client, _webhookUrl)) {
        http.addHeader("Content-Type", "application/json");

        char payload[260];
        snprintf(payload, sizeof(payload),
                 "{\"type\":\"boot\",\"token\":\"%s\",\"board\":\"%s\",\"reason\":\"%s\",\"uptime\":%lu}",
                 _token, BOARD_TARGET_NAME, reason ? reason : "Boot", (unsigned long)(millis() / 1000));

        int code = http.POST((uint8_t*)payload, strlen(payload));
        if (code > 0) {
            String resp = http.getString();
            handleServerResponse(resp);
        }
        http.end();
    }
}

void GoogleSheetClient::sendHeartbeat() {
    if (!isConfigured() || WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(8000);

    if (http.begin(client, _webhookUrl)) {
        http.addHeader("Content-Type", "application/json");

        char payload[550];
        if (_controller != nullptr) {
            const RFCommand& cUp = _controller->getCommand(ACTION_UP);
            const RFCommand& cDown = _controller->getCommand(ACTION_DOWN);
            const RFCommand& cStop = _controller->getCommand(ACTION_STOP);
            const RFCommand& cLock = _controller->getCommand(ACTION_LOCK);

            snprintf(payload, sizeof(payload),
                     "{\"type\":\"ping\",\"token\":\"%s\",\"board\":\"%s\",\"uptime\":%lu,"
                     "\"rfCodes\":["
                     "{\"action\":\"up\",\"hex\":\"0x%lX\",\"code\":%lu,\"bits\":%u,\"proto\":%u,\"pulse\":%u,\"isValid\":%s},"
                     "{\"action\":\"down\",\"hex\":\"0x%lX\",\"code\":%lu,\"bits\":%u,\"proto\":%u,\"pulse\":%u,\"isValid\":%s},"
                     "{\"action\":\"stop\",\"hex\":\"0x%lX\",\"code\":%lu,\"bits\":%u,\"proto\":%u,\"pulse\":%u,\"isValid\":%s},"
                     "{\"action\":\"lock\",\"hex\":\"0x%lX\",\"code\":%lu,\"bits\":%u,\"proto\":%u,\"pulse\":%u,\"isValid\":%s}]}",
                     _token, BOARD_TARGET_NAME, (unsigned long)(millis() / 1000),
                     (unsigned long)cUp.code, (unsigned long)cUp.code, cUp.bitLength > 0 ? cUp.bitLength : 24, cUp.protocol > 0 ? cUp.protocol : 1, cUp.pulseLength > 0 ? cUp.pulseLength : 350, cUp.isValid ? "true" : "false",
                     (unsigned long)cDown.code, (unsigned long)cDown.code, cDown.bitLength > 0 ? cDown.bitLength : 24, cDown.protocol > 0 ? cDown.protocol : 1, cDown.pulseLength > 0 ? cDown.pulseLength : 350, cDown.isValid ? "true" : "false",
                     (unsigned long)cStop.code, (unsigned long)cStop.code, cStop.bitLength > 0 ? cStop.bitLength : 24, cStop.protocol > 0 ? cStop.protocol : 1, cStop.pulseLength > 0 ? cStop.pulseLength : 350, cStop.isValid ? "true" : "false",
                     (unsigned long)cLock.code, (unsigned long)cLock.code, cLock.bitLength > 0 ? cLock.bitLength : 24, cLock.protocol > 0 ? cLock.protocol : 1, cLock.pulseLength > 0 ? cLock.pulseLength : 350, cLock.isValid ? "true" : "false");
        } else {
            snprintf(payload, sizeof(payload),
                     "{\"type\":\"ping\",\"token\":\"%s\",\"board\":\"%s\",\"uptime\":%lu}",
                     _token, BOARD_TARGET_NAME, (unsigned long)(millis() / 1000));
        }

        int code = http.POST((uint8_t*)payload, strlen(payload));
        if (code > 0) {
            String resp = http.getString();
            handleServerResponse(resp);
        }
        http.end();
    }
}

void GoogleSheetClient::executePost() {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000);

    if (http.begin(client, _webhookUrl)) {
        http.addHeader("Content-Type", "application/json");

        char payload[360];
        snprintf(payload, sizeof(payload),
                 "{\"type\":\"event\",\"token\":\"%s\",\"board\":\"%s\",\"event\":\"%s\",\"doorState\":\"%s\",\"openDurationSeconds\":%lu,\"source\":\"%s\"}",
                 _token, BOARD_TARGET_NAME, _pending.event, _pending.doorState, (unsigned long)_pending.durationSec, _pending.source);

        int httpCode = http.POST((uint8_t*)payload, strlen(payload));

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == 302) {
            _pending.isPending = false;
            String resp = http.getString();
            handleServerResponse(resp);
        }
        http.end();
    }
}

void GoogleSheetClient::handleServerResponse(const String& payload) {
    if (_controller == nullptr || payload.length() == 0) return;

    int idx = payload.indexOf("\"pendingCommand\":\"");
    if (idx < 0) return;

    int start = idx + 18;
    int end = payload.indexOf("\"", start);
    if (end < 0) return;

    String cmd = payload.substring(start, end);
    cmd.trim();
    if (cmd.length() == 0) return;

    Serial.print(F("[CLOUD SYNC] Nhan lenh tu Google Sheet: "));
    Serial.println(cmd);

    if (cmd == "up") {
        _controller->sendCommand(ACTION_UP);
    } else if (cmd == "down") {
        _controller->sendCommand(ACTION_DOWN);
    } else if (cmd == "stop") {
        _controller->sendCommand(ACTION_STOP);
    } else if (cmd == "lock") {
        _controller->sendCommand(ACTION_LOCK);
    } else if (cmd == "learn_up") {
        _controller->startLearning(ACTION_UP, 15000);
    } else if (cmd == "learn_down") {
        _controller->startLearning(ACTION_DOWN, 15000);
    } else if (cmd == "learn_stop") {
        _controller->startLearning(ACTION_STOP, 15000);
    } else if (cmd == "learn_lock") {
        _controller->startLearning(ACTION_LOCK, 15000);
    } else if (cmd.startsWith("set_")) {
        char buf[96] = {0};
        strncpy(buf, cmd.c_str(), sizeof(buf) - 1);
        char* token = strtok(buf, "_");
        char* actStr = strtok(nullptr, "_");
        char* hexStr = strtok(nullptr, "_");
        char* bitsStr = strtok(nullptr, "_");
        char* protoStr = strtok(nullptr, "_");
        char* pulseStr = strtok(nullptr, "_");

        DoorAction act;
        if (actStr && hexStr && parseAction(actStr, act)) {
            uint32_t code = strtoul(hexStr, nullptr, 0);
            uint8_t bits = bitsStr ? (uint8_t)atoi(bitsStr) : 24;
            uint8_t proto = protoStr ? (uint8_t)atoi(protoStr) : 1;
            uint16_t pulse = pulseStr ? (uint16_t)atoi(pulseStr) : 350;

            _controller->setCommand(act, RFCommand(code, bits, proto, pulse));
            _controller->save();

            Serial.print(F("[CLOUD SYNC] Da cap nhat thanh cong ma RF nut "));
            Serial.print(actStr);
            Serial.print(F(" thanh: "));
            Serial.print(hexStr);
            Serial.print(F(" ("));
            Serial.print(code);
            Serial.println(F(")"));
        }
    } else if (cmd.startsWith("travel_")) {
        uint16_t sec = cmd.substring(7).toInt();
        if (sec >= 5 && sec <= 120) {
            SystemConfig cfg;
            _controller->getStorage().loadSystemConfig(cfg);
            cfg.travelTimeSec = sec;
            _controller->getStorage().saveSystemConfig(cfg);
            Serial.print(F("[CLOUD SYNC] Cap nhat thoi gian hanh trinh: "));
            Serial.println(sec);
        }
    } else if (cmd.startsWith("clear_")) {
        String actStr = cmd.substring(6);
        DoorAction act;
        if (parseAction(actStr.c_str(), act)) {
            RFCommand emptyCmd;
            _controller->setCommand(act, emptyCmd);
            _controller->save();
            Serial.print(F("[CLOUD SYNC] Da xoa ma RF cua nut: "));
            Serial.println(actStr);
        }
    } else if (cmd.startsWith("import_")) {
        String str = cmd.substring(7);
        char buf[128] = {0};
        strncpy(buf, str.c_str(), sizeof(buf) - 1);
        char* token = strtok(buf, " ");
        if (token) {
            uint32_t cUp = strtoul(token, nullptr, 0);
            token = strtok(nullptr, " ");
            if (token) {
                uint32_t cDown = strtoul(token, nullptr, 0);
                token = strtok(nullptr, " ");
                if (token) {
                    uint32_t cStop = strtoul(token, nullptr, 0);
                    token = strtok(nullptr, " ");
                    if (token) {
                        uint32_t cLock = strtoul(token, nullptr, 0);
                        uint8_t bits = 24, proto = 1; uint16_t pulse = 350;
                        token = strtok(nullptr, " "); if (token) bits = atoi(token);
                        token = strtok(nullptr, " "); if (token) proto = atoi(token);
                        token = strtok(nullptr, " "); if (token) pulse = atoi(token);
                        _controller->setCommand(ACTION_UP, RFCommand(cUp, bits, proto, pulse));
                        _controller->setCommand(ACTION_DOWN, RFCommand(cDown, bits, proto, pulse));
                        _controller->setCommand(ACTION_STOP, RFCommand(cStop, bits, proto, pulse));
                        _controller->setCommand(ACTION_LOCK, RFCommand(cLock, bits, proto, pulse));
                        _controller->save();
                    }
                }
            }
        }
    }
}
