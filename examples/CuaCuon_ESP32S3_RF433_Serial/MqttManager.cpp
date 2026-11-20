#include "MqttManager.h"
#include "RollingDoorController.h"
#include "DoorMotionTracker.h"

MqttManager::MqttManager(RollingDoorController& controller, DoorMotionTracker& tracker)
    : _controller(controller),
      _tracker(tracker),
      _lastReconnectAttempt(0),
      _isTls(false) {
    memset(&_config, 0, sizeof(_config));
}

MqttManager::~MqttManager() {
}

bool MqttManager::begin(const SystemConfig& config) {
    _config = config;
    setupClient();
    return true;
}

void MqttManager::setConfig(const SystemConfig& config) {
    bool changed = (_config.mqttEnabled != config.mqttEnabled) ||
                   (strcmp(_config.mqttServer, config.mqttServer) != 0) ||
                   (_config.mqttPort != config.mqttPort) ||
                   (strcmp(_config.mqttUser, config.mqttUser) != 0) ||
                   (strcmp(_config.mqttPass, config.mqttPass) != 0) ||
                   (strcmp(_config.mqttTopic, config.mqttTopic) != 0);

    _config = config;

    if (changed) {
        if (_mqtt.connected()) {
            _mqtt.disconnect();
        }
        _lastReconnectAttempt = 0;
        setupClient();
    }
}

void MqttManager::setupClient() {
    if (!_config.mqttEnabled || strlen(_config.mqttServer) == 0) {
        return;
    }

    _isTls = (_config.mqttPort == 8883);

    if (_isTls) {
#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
        _tlsClient.setInsecure();
        _tlsClient.setBufferSizes(1024, 1024);
#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        _tlsClient.setInsecure();
#endif
        _mqtt.setClient(_tlsClient);
    } else {
        _mqtt.setClient(_tcpClient);
    }

    _mqtt.setServer(_config.mqttServer, _config.mqttPort);
    _mqtt.setBufferSize(512);
    _mqtt.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
        this->onMessageReceived(topic, payload, length);
    });

    Serial.print(F("[MQTT] Da thiet lap Broker: "));
    Serial.print(_config.mqttServer);
    Serial.print(F(":"));
    Serial.print(_config.mqttPort);
    Serial.println(_isTls ? F(" (Bao mat SSL/TLS)") : F(" (Non-TLS)"));
}

void MqttManager::update() {
    if (!_config.mqttEnabled || strlen(_config.mqttServer) == 0) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!_mqtt.connected()) {
        uint32_t now = millis();
        if (_lastReconnectAttempt == 0 || (now - _lastReconnectAttempt >= 10000)) {
            _lastReconnectAttempt = now;
            tryReconnect();
        }
    } else {
        _mqtt.loop();
    }
}

void MqttManager::tryReconnect() {
    Serial.print(F("[MQTT] Dang ket noi toi Broker: "));
    Serial.print(_config.mqttServer);
    Serial.println(F("..."));

    // Tạo Client ID định danh duy nhất
    char clientId[40];
    snprintf(clientId, sizeof(clientId), "HoaBinhDoor-%05ld", (long)random(10000, 99999));

    // Thiết lập Last Will and Testament (LWT) để Broker tự động báo offline nếu ESP mất nguồn
    char willTopic[80];
    snprintf(willTopic, sizeof(willTopic), "%s/status", _config.mqttTopic);
    const char* willPayload = "offline";

    bool ok = false;
    if (strlen(_config.mqttUser) > 0) {
        ok = _mqtt.connect(clientId, _config.mqttUser, _config.mqttPass, willTopic, 0, true, willPayload);
    } else {
        ok = _mqtt.connect(clientId, willTopic, 0, true, willPayload);
    }

    if (ok) {
        Serial.println(F("[MQTT THANH CONG] Da ket noi thanh cong Broker!"));

        // 1. Phát trạng thái sống online (retained)
        _mqtt.publish(willTopic, "online", true);

        // 2. Đăng ký lắng nghe kênh lệnh điều khiển từ xa: <topic>/command
        char cmdTopic[80];
        snprintf(cmdTopic, sizeof(cmdTopic), "%s/command", _config.mqttTopic);
        _mqtt.subscribe(cmdTopic);

        Serial.print(F("[MQTT] Da dang ky nhan lenh tu: "));
        Serial.println(cmdTopic);

        // 3. Phát ngay trạng thái hiện tại của cửa cuốn lên Broker
        publishState("MQTT Connected");
    } else {
        Serial.print(F("[MQTT THAT BAI] Khong the ket noi, rc="));
        Serial.print(_mqtt.state());
        Serial.println(F(". Se thu lai sau 10s."));
    }
}

void MqttManager::onMessageReceived(char* topic, uint8_t* payload, unsigned int length) {
    if (length == 0 || length >= 64) return;

    char cmd[64];
    memcpy(cmd, payload, length);
    cmd[length] = '\0';

    // Loại bỏ khoảng trắng thừa
    char* trimmed = cmd;
    while (*trimmed == ' ' || *trimmed == '\r' || *trimmed == '\n') trimmed++;
    int len = strlen(trimmed);
    while (len > 0 && (trimmed[len - 1] == ' ' || trimmed[len - 1] == '\r' || trimmed[len - 1] == '\n')) {
        trimmed[--len] = '\0';
    }

    Serial.print(F("[MQTT LENH NHAN] Topic: "));
    Serial.print(topic);
    Serial.print(F(" | Lenh: '"));
    Serial.print(trimmed);
    Serial.println(F("'"));

    bool handled = false;
    if (strcasecmp(trimmed, "up") == 0 || strcmp(trimmed, "1") == 0) {
        handled = _controller.sendCommand(ACTION_UP);
        _tracker.triggerAction(ACTION_UP, "MQTT Cloud");
    } else if (strcasecmp(trimmed, "down") == 0 || strcmp(trimmed, "2") == 0) {
        handled = _controller.sendCommand(ACTION_DOWN);
        _tracker.triggerAction(ACTION_DOWN, "MQTT Cloud");
    } else if (strcasecmp(trimmed, "stop") == 0 || strcmp(trimmed, "3") == 0) {
        handled = _controller.sendCommand(ACTION_STOP);
        _tracker.triggerAction(ACTION_STOP, "MQTT Cloud");
    } else if (strcasecmp(trimmed, "lock") == 0 || strcmp(trimmed, "4") == 0) {
        handled = _controller.sendCommand(ACTION_LOCK);
        _tracker.triggerAction(ACTION_LOCK, "MQTT Cloud");
    } else if (strcasecmp(trimmed, "status") == 0 || strcasecmp(trimmed, "get") == 0) {
        publishState("MQTT Query");
        return;
    }

    if (handled) {
        Serial.println(F("[MQTT] Da phat tin hieu RF dieu khien cua cuon thanh cong!"));
        publishState("MQTT Triggered");
    } else {
        Serial.println(F("[MQTT CANH BAO] Nut nay chua duoc hoc ma RF hoac lenh khong hop le."));
    }
}

void MqttManager::publishState(const char* source) {
    if (!_mqtt.connected()) return;

    char stateTopic[80];
    snprintf(stateTopic, sizeof(stateTopic), "%s/state", _config.mqttTopic);

    char json[320];
    snprintf(json, sizeof(json),
             "{\"state\":\"%s\",\"stateCode\":%u,\"percentage\":%u,\"openDuration\":%lu,\"travelTime\":%u,\"source\":\"%s\",\"board\":\"%s\"}",
             _tracker.getStateNameVi(),
             (unsigned int)_tracker.getState(),
             (unsigned int)_tracker.getOpenPercentage(),
             (unsigned long)_tracker.getCurrentOpenDurationSec(),
             (unsigned int)_tracker.getTravelTimeSec(),
             source ? source : _tracker.getLastSource(),
             BOARD_TARGET_NAME);

    _mqtt.publish(stateTopic, json, false);
}

bool MqttManager::isConnected() {
    return _mqtt.connected();
}
