#include <iostream>
#include <libyasdimaster.h>
#include <cassert>
#include <cstring>

#include "Channel.h"
#include "Device.h"

using namespace std;
using namespace yasdi;

Channel::Channel(DWORD handle) noexcept :
        _handle(handle),
        _updating(false) {
    constexpr size_t len = 32;
    char buf[len];
    GetChannelName(_handle, buf, len);
    _name = buf;
    _name.shrink_to_fit();
    GetChannelUnit(_handle, buf, len);
    _unit = buf;
    _unit.shrink_to_fit();
}

bool Channel::update(Device &device, DWORD maxAgeSeconds) {
    switch (GetChannelValueAsync(_handle, device.handle(), maxAgeSeconds)) {
        case YE_SHUTDOWN:
            break;
        case YE_UNKNOWN_HANDLE:
            break;
        case YE_NO_ACCESS_RIGHTS:
            break;
        case YE_TOO_MANY_REQUESTS:
            break;
        case YE_OK:
            _updating = true;
            break;
        default:
            break;
    }
    return this->isUpdating();
}

void Channel::onChannelValueReceived(Device &device, double value, char *valueText, int errorCode) {
    {
        switch (errorCode) {
            case YE_TIMEOUT:
                device.onChannelTimeout(*this);
                _valueValid = false;
                break;
            case YE_VALUE_NOT_VALID:
                _valueValid = false;
                break;
            case YE_OK:
                _value = value;
                _valueText = valueText;
                _timestamp = GetChannelValueTimeStamp(this->handle(), device.handle());
                _valueValid = true;
                break;
        }
        _updating = false;
    }
    device.onChannelUpdated(*this);
}

DWORD Channel::handle() const {
    return _handle;
}

double Channel::value() const {
    return _value;
}

string Channel::valueText() const {
    return _valueText;
}

const string &Channel::name() const {
    return _name;
}

bool Channel::isUpdating() const {
    return _updating;
}

bool Channel::isValid() const {
    return _valueValid;
}

string Channel::unit() const {
    return _unit;
}

DWORD Channel::timestamp() const {
    return _timestamp;
}
