#include "DoorMotionTracker.h"

DoorMotionTracker::DoorMotionTracker()
    : _state(MOTION_CLOSED),
      _travelTimeSec(DEFAULT_TRAVEL_TIME_SEC),
      _currentPercentage(0),
      _travelStartTime(0),
      _openCycleStartTime(0),
      _lastTotalOpenDurationSec(0),
      _lastActionTime(0),
      _onMotionChangeCb(nullptr) {
    memset(_lastSource, 0, sizeof(_lastSource));
    strncpy(_lastSource, "KhoiTao", sizeof(_lastSource) - 1);
}

DoorMotionTracker::~DoorMotionTracker() {
}

bool DoorMotionTracker::begin(uint16_t travelTimeSec) {
    if (travelTimeSec > 0 && travelTimeSec < 120) {
        _travelTimeSec = travelTimeSec;
    }
    _state = MOTION_CLOSED;
    _currentPercentage = 0;
    return true;
}

void DoorMotionTracker::update() {
    uint32_t now = millis();

    if (_state == MOTION_OPENING) {
        uint32_t elapsed = now - _travelStartTime;
        uint32_t totalMs = (uint32_t)_travelTimeSec * 1000;

        if (elapsed >= totalMs) {
            _currentPercentage = 100;
            setState(MOTION_OPENED, _lastSource);
        } else {
            _currentPercentage = (uint8_t)((elapsed * 100) / totalMs);
        }
    }
    else if (_state == MOTION_CLOSING) {
        uint32_t elapsed = now - _travelStartTime;
        uint32_t totalMs = (uint32_t)_travelTimeSec * 1000;

        if (elapsed >= totalMs) {
            _currentPercentage = 0;
            if (_openCycleStartTime > 0) {
                _lastTotalOpenDurationSec = (now - _openCycleStartTime) / 1000;
                _openCycleStartTime = 0;
            }
            setState(MOTION_CLOSED, _lastSource);
        } else {
            uint8_t closedPct = (uint8_t)((elapsed * 100) / totalMs);
            _currentPercentage = (closedPct >= 100) ? 0 : (100 - closedPct);
        }
    }
}

void DoorMotionTracker::triggerAction(DoorAction action, const char* source) {
    if (source) {
        strncpy(_lastSource, source, sizeof(_lastSource) - 1);
        _lastSource[sizeof(_lastSource) - 1] = '\0';
    }
    _lastActionTime = millis();

    switch (action) {
        case ACTION_UP:
            if (_state != MOTION_OPENED && _state != MOTION_OPENING) {
                if (_state == MOTION_CLOSED || _openCycleStartTime == 0) {
                    _openCycleStartTime = millis();
                }
                _travelStartTime = millis();
                setState(MOTION_OPENING, _lastSource);
            }
            break;

        case ACTION_DOWN:
            if (_state != MOTION_CLOSED && _state != MOTION_CLOSING) {
                _travelStartTime = millis();
                setState(MOTION_CLOSING, _lastSource);
            }
            break;

        case ACTION_STOP:
            if (_state == MOTION_OPENING || _state == MOTION_CLOSING) {
                setState(MOTION_STOPPED, _lastSource);
            }
            break;

        case ACTION_LOCK:
            setState(MOTION_LOCKED, _lastSource);
            break;

        default:
            break;
    }
}

void DoorMotionTracker::setTravelTimeSec(uint16_t sec) {
    if (sec >= 5 && sec <= 120) {
        _travelTimeSec = sec;
    }
}

uint32_t DoorMotionTracker::getCurrentOpenDurationSec() const {
    if (_openCycleStartTime == 0 || _state == MOTION_CLOSED) {
        return 0;
    }
    return (millis() - _openCycleStartTime) / 1000;
}

const char* DoorMotionTracker::getStateNameVi() const {
    switch (_state) {
        case MOTION_CLOSED:   return "ĐÃ ĐÓNG KÍN";
        case MOTION_OPENING:  return "ĐANG MỞ LÊN...";
        case MOTION_OPENED:   return "ĐÃ MỞ HOÀN TOÀN";
        case MOTION_CLOSING:  return "ĐANG ĐÓNG XUỐNG...";
        case MOTION_STOPPED:  return "TẠM DỪNG LƠ LỬNG";
        case MOTION_LOCKED:   return "ĐANG KHÓA HỘP";
        default:              return "KHÔNG XÁC ĐỊNH";
    }
}

void DoorMotionTracker::setState(MotionState newState, const char* source) {
    _state = newState;
    if (_onMotionChangeCb) {
        uint32_t duration = (_state == MOTION_CLOSED) ? _lastTotalOpenDurationSec : getCurrentOpenDurationSec();
        _onMotionChangeCb(_state, getStateNameVi(), duration, source);
    }
}
