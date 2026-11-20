#include "RollingDoorController.h"

RollingDoorController::RollingDoorController()
    : _state(STATE_IDLE),
      _learnAction(ACTION_UP),
      _learnStartTime(0),
      _learnTimeoutMs(DEFAULT_LEARN_TIMEOUT),
      _lastCaptureTime(0),
      _ledPin(-1),
      _lastLedToggleTime(0),
      _ledState(false),
      _snifferMode(false),
      _lastLearnSuccess(false),
      _lastLearnFinished(false),
      _onLearnCb(nullptr),
      _onActionCb(nullptr),
      _onRemoteReceivedCb(nullptr),
      _onSniffCb(nullptr),
      _onStateChangeCb(nullptr) {
}

RollingDoorController::~RollingDoorController() {
}

bool RollingDoorController::begin(int8_t rxPin, int8_t txPin, int8_t ledPin) {
    _ledPin = ledPin;
    if (_ledPin >= 0) {
        pinMode(_ledPin, OUTPUT);
        digitalWrite(_ledPin, LOW);
    }

    // Khởi tạo bộ thu phát RF 433
    if (!_rf.begin(rxPin, txPin, DEFAULT_TX_REPEATS)) {
        return false;
    }

    // Khởi tạo bộ nhớ lưu trữ
    if (_storage.begin()) {
        if (_storage.hasSavedData()) {
            // Tải các lệnh đã lưu trước đó trong Flash
            _storage.loadAll(_commands);
        } else if (PRESET_CODE_UP != 0) {
            // Tự động nạp mã cấu hình sẵn từ Config.h (nếu có khai báo)
            _commands[ACTION_UP]   = RFCommand(PRESET_CODE_UP, PRESET_BIT_LENGTH, PRESET_PROTOCOL, PRESET_PULSE_LENGTH);
            _commands[ACTION_DOWN] = RFCommand(PRESET_CODE_DOWN, PRESET_BIT_LENGTH, PRESET_PROTOCOL, PRESET_PULSE_LENGTH);
            _commands[ACTION_STOP] = RFCommand(PRESET_CODE_STOP, PRESET_BIT_LENGTH, PRESET_PROTOCOL, PRESET_PULSE_LENGTH);
            _commands[ACTION_LOCK] = RFCommand(PRESET_CODE_LOCK, PRESET_BIT_LENGTH, PRESET_PROTOCOL, PRESET_PULSE_LENGTH);
            _storage.saveAll(_commands);
        }
    }

    setState(STATE_IDLE, "Ready");
    return true;
}

void RollingDoorController::update() {
    uint32_t now = millis();

    // Quản lý LED chỉ thị trạng thái
    updateLed();

    // Xử lý State Machine
    if (_state == STATE_LEARNING) {
        // Kiểm tra Hết hạn thời gian chờ học lệnh (Timeout)
        if (now - _learnStartTime >= _learnTimeoutMs) {
            DoorAction timedOutAction = _learnAction;
            _lastLearnFinished = true;
            _lastLearnSuccess = false;
            setState(STATE_IDLE, "Learn Timeout");

            if (_onLearnCb) {
                RFCommand emptyCmd;
                _onLearnCb(timedOutAction, emptyCmd, false);
            }
            return;
        }

        // Kiểm tra có tín hiệu RF nhận được
        if (_rf.isReceiverAvailable()) {
            RFCommand receivedCmd;
            if (_rf.readReceivedCommand(receivedCmd)) {
                // Kiểm tra chống dội (debounce)
                if (now - _lastCaptureTime > DEBOUNCE_LEARN_MS) {
                    _lastCaptureTime = now;

                    // Lưu lệnh vừa học vào mảng tạm thời
                    _commands[_learnAction] = receivedCmd;
                    _storage.setModified(true);
                    _storage.saveAll(_commands); // Tự động lưu vĩnh viễn vào Flash ngay khi học thành công

                    _lastLearnFinished = true;
                    _lastLearnSuccess = true;
                    _lastLearnedCmd = receivedCmd;

                    DoorAction learnedAction = _learnAction;
                    setState(STATE_IDLE, "Learn Success");

                    // Bật còi / LED hoặc gọi callback
                    if (_onLearnCb) {
                        _onLearnCb(learnedAction, receivedCmd, true);
                    }
                }
            }
        }
    } else if (_snifferMode && _state == STATE_IDLE) {
        // Chế độ Sniffer: bắt và hiển thị tất cả các gói RF 433MHz quét được
        if (_rf.isReceiverAvailable()) {
            RFCommand sniffCmd;
            if (_rf.readReceivedCommand(sniffCmd)) {
                if (_onSniffCb) {
                    _onSniffCb(sniffCmd);
                }
            }
        }
    } else {
        // Trạng thái bình thường: Luôn lắng nghe sóng RF trong không khí (Bắt mọi remote bấm ngoài nhà)
        if (_rf.isReceiverAvailable()) {
            RFCommand ambientCmd;
            if (_rf.readReceivedCommand(ambientCmd)) {
                if (now - _lastCaptureTime > DEBOUNCE_LEARN_MS) {
                    _lastCaptureTime = now;
                    for (uint8_t i = 0; i < ACTION_COUNT; i++) {
                        if (_commands[i].isValid && _commands[i].code == ambientCmd.code) {
                            if (_onRemoteReceivedCb) {
                                _onRemoteReceivedCb((DoorAction)i, ambientCmd);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool RollingDoorController::startLearning(DoorAction action, uint32_t timeoutMs) {
    if (action >= ACTION_COUNT) return false;

    _learnAction = action;
    _learnTimeoutMs = timeoutMs;
    _learnStartTime = millis();
    _lastCaptureTime = 0;
    _lastLearnSuccess = false;
    _lastLearnFinished = false;

    _rf.resetReceiver();
    _rf.enableReceiver();

    setState(STATE_LEARNING, "Waiting for RF signal");
    return true;
}

void RollingDoorController::cancelLearning() {
    if (_state == STATE_LEARNING) {
        _lastLearnFinished = true;
        _lastLearnSuccess = false;
        setState(STATE_IDLE, "Learn Canceled");
    }
}

uint32_t RollingDoorController::getLearningRemainingMs() const {
    if (_state != STATE_LEARNING) return 0;
    uint32_t elapsed = millis() - _learnStartTime;
    if (elapsed >= _learnTimeoutMs) return 0;
    return (_learnTimeoutMs - elapsed);
}

bool RollingDoorController::sendCommand(DoorAction action) {
    if (action >= ACTION_COUNT) return false;
    if (!_commands[action].isValid) return false;

    return sendRawCommand(_commands[action]);
}

bool RollingDoorController::sendRawCommand(const RFCommand& cmd, uint8_t repeats) {
    if (!cmd.isValid || cmd.code == 0) return false;

    setState(STATE_TRANSMITTING, "Transmitting RF");

    if (_ledPin >= 0) {
        digitalWrite(_ledPin, HIGH);
    }

    _rf.sendCommand(cmd, repeats);

    if (_ledPin >= 0) {
        digitalWrite(_ledPin, LOW);
    }

    setState(STATE_IDLE, "Transmitted");

    if (_onActionCb) {
        // Tìm xem lệnh này thuộc nút nào (nếu có)
        for (uint8_t i = 0; i < ACTION_COUNT; i++) {
            if (_commands[i].isValid && _commands[i].code == cmd.code) {
                _onActionCb((DoorAction)i, cmd);
                break;
            }
        }
    }

    return true;
}

bool RollingDoorController::save() {
    return _storage.saveAll(_commands);
}

bool RollingDoorController::load() {
    return _storage.loadAll(_commands);
}

bool RollingDoorController::clear() {
    for (uint8_t i = 0; i < ACTION_COUNT; i++) {
        _commands[i] = RFCommand();
    }
    return _storage.clear();
}

const RFCommand& RollingDoorController::getCommand(DoorAction action) const {
    static const RFCommand invalidCmd;
    if (action >= ACTION_COUNT) return invalidCmd;
    return _commands[action];
}

bool RollingDoorController::setCommand(DoorAction action, const RFCommand& cmd) {
    if (action >= ACTION_COUNT) return false;
    _commands[action] = cmd;
    _storage.setModified(true);
    return true;
}

bool RollingDoorController::hasValidCommand(DoorAction action) const {
    if (action >= ACTION_COUNT) return false;
    return _commands[action].isValid && (_commands[action].code != 0);
}

void RollingDoorController::setSnifferMode(bool enabled) {
    _snifferMode = enabled;
    if (_snifferMode) {
        _rf.enableReceiver();
        _rf.resetReceiver();
    }
}

void RollingDoorController::setState(ControllerState newState, const char* reason) {
    _state = newState;
    if (_onStateChangeCb) {
        _onStateChangeCb(_state, reason ? reason : "");
    }
}

void RollingDoorController::updateLed() {
    if (_ledPin < 0) return;

    uint32_t now = millis();
    if (_state == STATE_LEARNING) {
        // Nhấp nháy nhanh khi đang học lệnh (150ms)
        if (now - _lastLedToggleTime >= 150) {
            _lastLedToggleTime = now;
            _ledState = !_ledState;
            digitalWrite(_ledPin, _ledState ? HIGH : LOW);
        }
    } else if (_state == STATE_TRANSMITTING) {
        digitalWrite(_ledPin, HIGH);
    } else {
        // Trạng thái nghỉ: tắt LED
        if (_ledState) {
            _ledState = false;
            digitalWrite(_ledPin, LOW);
        }
    }
}
