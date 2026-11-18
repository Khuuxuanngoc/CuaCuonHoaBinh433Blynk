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

bool StorageManager::begin() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    // Mở namespace "cuacuon" ở chế độ read/write
    bool ok = s_prefs.begin(NVS_NAMESPACE, false);
    if (!ok) return false;

    // Kiểm tra xem đã có key lưu dữ liệu hay chưa
    _hasSavedData = s_prefs.isKey("rf_cmds");
    return true;

#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
    EEPROM.begin(EEPROM_SIZE_BYTES);

    EEPROMRecord record;
    EEPROM.get(EEPROM_START_ADDR, record);

    if (record.magic == EEPROM_MAGIC_KEY && record.checksum == calculateChecksum(record)) {
        _hasSavedData = true;
    } else {
        _hasSavedData = false;
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
    loadAll(temp); // Đọc mảng hiện tại ra
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
    memset(&record, 0, sizeof(record));
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
    size_t len = sizeof(record.magic) + sizeof(record.commands);
    uint16_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum = (sum << 1) ^ p[i];
    }
    return sum;
}
#endif
