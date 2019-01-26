#include <cassert>
#include <libyasdimaster.h>
#include <iostream>
#include <algorithm>
#include "Channel.h"
#include "Device.h"
#include "Master.h"

using namespace yasdi;
using namespace std;

Device::Device(yasdi::Master &master, DWORD deviceHandle) noexcept
        : _master(master),
          _handle(deviceHandle),
          _online(true),
          _initialized(false),
          _channelUpdatesRemaining(0) {
}

void Device::init() {
    bool initSuccess = true;
    constexpr unsigned short maxLen = 64;
    char name[maxLen];
    int rc = GetDeviceName(_handle, name, maxLen);
    // this assert fails wtf?! should be the string length, see YASDI docs
    //assert(rc > 0);
    _name = name;
    _name.shrink_to_fit();
    DWORD sn;
    rc = GetDeviceSN(_handle, &sn);
//    assert(YE_OK == rc);
    _serialNumber = sn;
    rc = GetDeviceType(_handle, name, maxLen);
//    assert(YE_OK == rc);
    _type = name;
    _type.shrink_to_fit();
    constexpr unsigned short maxChannels = 50;
    DWORD channelHandles[maxChannels];
    auto channelsCount = GetChannelHandlesEx(handle(), channelHandles, maxChannels, SPOTCHANNELS);
    for (auto i = 0; i < channelsCount; i++) {
        auto channel = make_shared<Channel>(channelHandles[i]);
        _channels.push_back(channel);
    }
    _channels.shrink_to_fit();
    _initialized = initSuccess;
}

void Device::updateChannels(DWORD maxAgeSeconds) {
    for (auto &channel : _channels) {
        if (channel->update(*this, maxAgeSeconds)) {
            _channelUpdatesRemaining++;
        }
    }
}

void Device::onChannelValueReceived(DWORD channelHandle, double value, char *valueText, int errorCode) {
    //assert(this->isUpdating());
    auto channel = find_if(_channels.begin(), _channels.end(), [channelHandle](shared_ptr<Channel> const &channel) {
        return channel->handle() == channelHandle;
    });
//    assert(channel != _channels.end());
    (*channel)->onChannelValueReceived(*this, value, valueText, errorCode);
}

void Device::onChannelUpdated(const Channel &channel) {
    _channelUpdatesRemaining--;
    if (!this->isUpdating()) {
        _master.onDeviceUpdated(*this);
    }
}

void Device::onChannelTimeout(const Channel &channel) {
    if (_online) {
        _online = false;
        _master.onDeviceOffline(*this);
    }
}

const vector<shared_ptr<Channel>> &Device::channels() const {
    return _channels;
}

const bool Device::isUpdating() const {
    return _channelUpdatesRemaining > 0;
}

const bool Device::isOnline() const {
    return _initialized && _online;
}

const string Device::name() const {
    return _name;
}

const string Device::type() const {
    return _type;
}

const DWORD Device::handle() const {
    return _handle;
}

const DWORD Device::serialNumber() const {
    return _serialNumber;
}