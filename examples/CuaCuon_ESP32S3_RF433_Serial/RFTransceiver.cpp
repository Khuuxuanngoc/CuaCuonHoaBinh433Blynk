#include "RFTransceiver.h"

RFTransceiver::RFTransceiver()
    : _rxPin(-1), _txPin(-1), _txRepeats(10), _isRxEnabled(false) {
}

RFTransceiver::~RFTransceiver() {
    disableReceiver();
}

bool RFTransceiver::begin(int8_t rxPin, int8_t txPin, uint8_t defaultTxRepeats) {
    _rxPin = rxPin;
    _txPin = txPin;
    _txRepeats = defaultTxRepeats;

    if (_txPin >= 0) {
        pinMode(_txPin, OUTPUT);
        digitalWrite(_txPin, LOW);
        _rf.enableTransmit(_txPin);
        _rf.setRepeatTransmit(_txRepeats);
    }

    if (_rxPin >= 0) {
        pinMode(_rxPin, INPUT);
        enableReceiver();
    }

    return true;
}

void RFTransceiver::enableReceiver() {
    if (_rxPin < 0) return;

    int8_t interruptNum = digitalPinToInterrupt(_rxPin);
    if (interruptNum != NOT_AN_INTERRUPT) {
        _rf.enableReceive(interruptNum);
        _isRxEnabled = true;
    }
}

void RFTransceiver::disableReceiver() {
    if (_isRxEnabled) {
        _rf.disableReceive();
        _isRxEnabled = false;
    }
}

bool RFTransceiver::isReceiverAvailable() {
    return _isRxEnabled && _rf.available();
}

bool RFTransceiver::readReceivedCommand(RFCommand& outCmd) {
    if (!_isRxEnabled || !_rf.available()) {
        return false;
    }

    uint32_t val = (uint32_t)_rf.getReceivedValue();
    uint8_t bits = (uint8_t)_rf.getReceivedBitlength();
    uint16_t delay = (uint16_t)_rf.getReceivedDelay();
    uint8_t proto = (uint8_t)_rf.getReceivedProtocol();

    _rf.resetAvailable();

    if (val == 0) {
        return false; // Gói tin lỗi hoặc nhiễu không xác định
    }

    outCmd.code = val;
    outCmd.bitLength = bits;
    outCmd.protocol = proto;
    outCmd.pulseLength = delay;
    outCmd.isValid = true;

    return true;
}

void RFTransceiver::resetReceiver() {
    if (_isRxEnabled) {
        _rf.resetAvailable();
    }
}

void RFTransceiver::sendCommand(const RFCommand& cmd, uint8_t repeats) {
    if (!cmd.isValid || cmd.code == 0 || _txPin < 0) {
        return;
    }

    // Tạm ngắt bộ thu để tránh việc bộ thu nhận lại chính tín hiệu mình vừa phát (tự nhiễu)
    bool wasRx = _isRxEnabled;
    if (wasRx) {
        disableReceiver();
    }

    uint8_t repCount = (repeats > 0) ? repeats : _txRepeats;
    _rf.setProtocol(cmd.protocol);
    _rf.setPulseLength(cmd.pulseLength);
    _rf.setRepeatTransmit(repCount);
    _rf.send(cmd.code, cmd.bitLength);

    // Bật lại bộ thu sau khi phát xong
    if (wasRx) {
        enableReceiver();
        resetReceiver();
    }
}
