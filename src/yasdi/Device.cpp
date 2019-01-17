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
    GetDeviceName(_handle, name, maxLen);
    _name = name;
    _name.shrink_to_fit();
    DWORD sn;
    switch (GetDeviceSN(_handle, &sn)) {
        case YE_OK:
            _serialNumber = sn;
            break;
        default:
            _serialNumber = 0;
            initSuccess = false;
            break;
    }
    switch (GetDeviceType(_handle, name, maxLen)) {
        case YE_OK:
            _type = name;
            break;
        default:
            _type = "";
            break;
    }
    _type.shrink_to_fit();
    constexpr unsigned short maxChannels = 50;
    DWORD channelHandles[maxChannels];
    auto channelsCount = GetChannelHandlesEx(handle(), channelHandles, maxChannels, SPOTCHANNELS);
    for (auto i = 0; i < channelsCount; i++) {
        auto channel = make_shared<Channel>(*this, channelHandles[i]);
//        cout << "SMA Logger --- "
//             << "Device '" << this->name() << "': "
//             << "Discovered spot channel '" << channel->name() << "'." << endl;
        _channels.push_back(channel);
    }
    _channels.shrink_to_fit();
    _initialized = initSuccess;
}

void Device::onChannelValueReceived(DWORD channelHandle, double value, char *valueText, int errorCode) {
    auto channel = find_if(_channels.begin(), _channels.end(), [channelHandle](shared_ptr<Channel> const &channel) {
        return channel->handle() == channelHandle;
    });
    assert(channel != _channels.end());
    (*channel)->onChannelValueReceived(*this, value, valueText, errorCode);
}

void Device::updateChannels(DWORD maxAgeSeconds) {
    assert(!this->isUpdating());
    {
        // std::lock_guard<std::mutex> lock(_mutex);
        _channelUpdatesRemaining = _channels.size();
    }
//    cout << "SMA Logger --- "
//         << "Device '" << this->name() << "': Updating channels." << endl;
    for (auto &channel : _channels) {
        channel->update(maxAgeSeconds);
    }
}

void Device::onChannelUpdated(const Channel &channel) {
    {
        // std::lock_guard<std::mutex> lock(_mutex);
        _channelUpdatesRemaining--;
    }
    if (!this->isUpdating()) {
        _master.onDeviceUpdated(*this);
    }
}

void Device::onChannelTimeout(const yasdi::Channel &channel) {
    if (_online) {
        {
            // std::lock_guard<std::mutex> lock(_mutex);
            _online = false;
        }
        _master.onDeviceOffline(*this);
    }
}

std::vector<std::shared_ptr<Channel>> Device::channels() const {
    return _channels;
}

bool Device::isUpdating() const {
    return _channelUpdatesRemaining != 0;
}

bool Device::setStatus(bool isOnline) {
    {
        // std::lock_guard<std::mutex> lock(_mutex);
        _online = isOnline;
    }
    return this->isOnline();
}

bool Device::isOnline() const {
    return _initialized && _online;
}

string Device::name() const {
    return _name;
}

string Device::type() const {
    return _type;
}

DWORD Device::handle() const {
    return _handle;
}

DWORD Device::serialNumber() const {
    return _serialNumber;
}