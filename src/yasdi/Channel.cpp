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
    constexpr size_t maxLength = 32;
    char buffer[maxLength] = {0};
    GetChannelName(_handle, buffer, maxLength);
    _name = buffer;
    _name.shrink_to_fit();
    memset(buffer, 0, maxLength);
    GetChannelUnit(_handle, buffer, maxLength);
    _unit = buffer;
    _unit.shrink_to_fit();
}

const bool Channel::update(Device &device, DWORD maxAgeSeconds) {
    switch (GetChannelValueAsync(_handle, device.handle(), maxAgeSeconds)) {
        case YE_SHUTDOWN:
        case YE_UNKNOWN_HANDLE:
        case YE_NO_ACCESS_RIGHTS:
        case YE_TOO_MANY_REQUESTS:
            device.onChannelUpdated(*this, ChannelUpdate::FAILURE);
            break;
        case YE_OK:
            _updating = true;
            break;
        default:
            device.onChannelUpdated(*this, ChannelUpdate::FAILURE);
            break;
    }
    return isUpdating();
}

void Channel::onChannelValueReceived(Device &device, const double value, const char *valueText, const int errorCode) {
//    cout << "SMA Logger --- Channel: Device '"
//         << device.name() << "' received value for channel '"
//         << name() << "'." << endl;
    // default value
    ChannelUpdate updateResult = ChannelUpdate::FAILURE;
    switch (errorCode) {
        case YE_TIMEOUT:
            _valueValid = false;
            updateResult = ChannelUpdate::TIMEOUT;
            break;
        case YE_VALUE_NOT_VALID:
            _valueValid = false;
            updateResult = ChannelUpdate::FAILURE;
            break;
        case YE_OK:
            _value = value;
            _valueText = valueText;
            _timestamp = GetChannelValueTimeStamp(handle(), device.handle());
            _valueValid = true;
            updateResult = ChannelUpdate::SUCCESS;
            break;
    }
    device.onChannelUpdated(*this, updateResult);
    _updating = false;
}

const DWORD Channel::handle() const {
    return _handle;
}

const double Channel::value() const {
    return _value;
}

const string Channel::valueText() const {
    return _valueText;
}

const string &Channel::name() const {
    return _name;
}

const bool Channel::isUpdating() const {
    return _updating;
}

const bool Channel::isValid() const {
    return _valueValid;
}

const bool Channel::isNumeric() const {
    return _valueText.empty();
}

const string Channel::unit() const {
    return _unit;
}

const DWORD Channel::timestamp() const {
    return _timestamp;
}
