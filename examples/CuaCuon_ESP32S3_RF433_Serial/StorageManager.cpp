#include "StorageManager.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#include <Preferences.h>
static Preferences s_prefs;
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
#include <EEPROM.h>
#endif

StorageManager::StorageManager()
    : _hasSavedData(false), _isModified(false) {
}

StorageManager::~StorageManager() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    s_prefs.end();
#endif
}

void StorageManager::getDefaultSystemConfig(SystemConfig& config) {
    memset(&config, 0, sizeof(config));
    strncpy(config.wifiSSID, WIFI_STA_SSID, sizeof(config.wifiSSID) - 1);
    strncpy(config.wifiPass, WIFI_STA_PASS, sizeof(config.wifiPass) - 1);
    strncpy(config.gasWebhookUrl, GAS_WEBHOOK_URL, sizeof(config.gasWebhookUrl) - 1);
    strncpy(config.webPassword, WEB_APP_PASSWORD, sizeof(config.webPassword) - 1);
    strncpy(config.apPassword, WIFI_AP_PASS, sizeof(config.apPassword) - 1);
    config.travelTimeSec = DEFAULT_TRAVEL_TIME_SEC;
}

bool StorageManager::begin() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    bool ok = s_prefs.begin(NVS_NAMESPACE, false);
    if (!ok) return false;

    _hasSavedData = s_prefs.isKey("rf_cmds");

    // Nếu chưa khởi tạo cấu hình hệ thống lần nào thì nạp mặc định
    if (!s_prefs.isKey("cfg_init")) {
        SystemConfig defCfg;
        getDefaultSystemConfig(defCfg);
        saveSystemConfig(defCfg);
        s_prefs.putBool("cfg_init", true);
    }
    return true;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROM.begin(EEPROM_SIZE_BYTES);

    EEPROMRecord record;
    EEPROM.get(EEPROM_START_ADDR, record);

    if (record.magic == EEPROM_MAGIC_KEY && record.checksum == calculateChecksum(record)) {
        _hasSavedData = true;
    } else {
        _hasSavedData = false;
        // Khởi tạo mặc định nếu EEPROM chưa hợp lệ
        resetToFactoryDefaults();
    }
    return true;

#else
    return false;
#endif
}

bool StorageManager::saveAll(const RFCommand commands[ACTION_COUNT]) {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    size_t written = s_prefs.putBytes("rf_cmds", commands, sizeof(RFCommand) * ACTION_COUNT);
    if (written == sizeof(RFCommand) * ACTION_COUNT) {
        _hasSavedData = true;
        _isModified = false;
        return true;
    }
    return false;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROMRecord record;
    EEPROM.get(EEPROM_START_ADDR, record);
    record.magic = EEPROM_MAGIC_KEY;
    for (uint8_t i = 0; i < ACTION_COUNT; i++) {
        record.commands[i] = commands[i];
    }
    record.checksum = calculateChecksum(record);

    EEPROM.put(EEPROM_START_ADDR, record);
    bool ok = EEPROM.commit();
    if (ok) {
        _hasSavedData = true;
        _isModified = false;
    }
    return ok;

#else
    return false;
#endif
}

bool StorageManager::loadAll(RFCommand commands[ACTION_COUNT]) {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    if (!s_prefs.isKey("rf_cmds")) {
        return false;
    }

    size_t readLen = s_prefs.getBytes("rf_cmds", commands, sizeof(RFCommand) * ACTION_COUNT);
    if (readLen == sizeof(RFCommand) * ACTION_COUNT) {
        _hasSavedData = true;
        _isModified = false;
        return true;
    }
    return false;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROMRecord record;
    EEPROM.get(EEPROM_START_ADDR, record);

    if (record.magic == EEPROM_MAGIC_KEY && record.checksum == calculateChecksum(record)) {
        for (uint8_t i = 0; i < ACTION_COUNT; i++) {
            commands[i] = record.commands[i];
        }
        _hasSavedData = true;
        _isModified = false;
        return true;
    }
    return false;

#else
    return false;
#endif
}

bool StorageManager::saveCommand(DoorAction action, const RFCommand& cmd) {
    if (action >= ACTION_COUNT) return false;

    RFCommand temp[ACTION_COUNT];
    loadAll(temp);
    temp[action] = cmd;
    return saveAll(temp);
}

bool StorageManager::loadCommand(DoorAction action, RFCommand& cmd) {
    if (action >= ACTION_COUNT) return false;

    RFCommand temp[ACTION_COUNT];
    if (loadAll(temp)) {
        cmd = temp[action];
        return cmd.isValid;
    }
    return false;
}

bool StorageManager::clear() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    s_prefs.remove("rf_cmds");
    _hasSavedData = false;
    _isModified = false;
    return true;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROMRecord record;
    EEPROM.get(EEPROM_START_ADDR, record);
    memset(record.commands, 0, sizeof(record.commands));
    record.checksum = calculateChecksum(record);
    EEPROM.put(EEPROM_START_ADDR, record);
    bool ok = EEPROM.commit();
    _hasSavedData = false;
    _isModified = false;
    return ok;

#else
    return false;
#endif
}

bool StorageManager::saveSystemConfig(const SystemConfig& config) {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    s_prefs.putString("wifi_ssid", config.wifiSSID);
    s_prefs.putString("wifi_pass", config.wifiPass);
    s_prefs.putString("gas_url", config.gasWebhookUrl);
    s_prefs.putString("web_pass", config.webPassword);
    s_prefs.putString("ap_pass", config.apPassword);
    s_prefs.putUShort("travel_t", config.travelTimeSec);
    return true;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROMRecord record;
    EEPROM.get(EEPROM_START_ADDR, record);
    record.magic = EEPROM_MAGIC_KEY;
    record.config = config;
    record.checksum = calculateChecksum(record);

    EEPROM.put(EEPROM_START_ADDR, record);
    return EEPROM.commit();

#else
    return false;
#endif
}

bool StorageManager::loadSystemConfig(SystemConfig& config) {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    if (!s_prefs.isKey("cfg_init")) {
        getDefaultSystemConfig(config);
        return false;
    }

    getDefaultSystemConfig(config);
    String s;
    s = s_prefs.getString("wifi_ssid", config.wifiSSID);
    strncpy(config.wifiSSID, s.c_str(), sizeof(config.wifiSSID) - 1);

    s = s_prefs.getString("wifi_pass", config.wifiPass);
    strncpy(config.wifiPass, s.c_str(), sizeof(config.wifiPass) - 1);

    s = s_prefs.getString("gas_url", config.gasWebhookUrl);
    strncpy(config.gasWebhookUrl, s.c_str(), sizeof(config.gasWebhookUrl) - 1);

    s = s_prefs.getString("web_pass", config.webPassword);
    strncpy(config.webPassword, s.c_str(), sizeof(config.webPassword) - 1);

    s = s_prefs.getString("ap_pass", config.apPassword);
    strncpy(config.apPassword, s.c_str(), sizeof(config.apPassword) - 1);

    config.travelTimeSec = s_prefs.getUShort("travel_t", config.travelTimeSec);
    return true;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROMRecord record;
    EEPROM.get(EEPROM_START_ADDR, record);

    if (record.magic == EEPROM_MAGIC_KEY && record.checksum == calculateChecksum(record)) {
        config = record.config;
        return true;
    }
    getDefaultSystemConfig(config);
    return false;

#else
    getDefaultSystemConfig(config);
    return false;
#endif
}

bool StorageManager::resetToFactoryDefaults() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    s_prefs.clear();
    SystemConfig defCfg;
    getDefaultSystemConfig(defCfg);
    saveSystemConfig(defCfg);
    s_prefs.putBool("cfg_init", true);
    _hasSavedData = false;
    _isModified = false;
    return true;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROMRecord record;
    memset(&record, 0, sizeof(record));
    record.magic = EEPROM_MAGIC_KEY;
    getDefaultSystemConfig(record.config);
    record.checksum = calculateChecksum(record);

    EEPROM.put(EEPROM_START_ADDR, record);
    bool ok = EEPROM.commit();
    _hasSavedData = false;
    _isModified = false;
    return ok;

#else
    return false;
#endif
}

#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
uint16_t StorageManager::calculateChecksum(const EEPROMRecord& record) {
    const uint8_t* p = (const uint8_t*)&record;
    size_t len = sizeof(record.magic) + sizeof(record.commands) + sizeof(record.config);
    uint16_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum = (sum << 1) ^ p[i];
    }
    return sum;
}
#endif
