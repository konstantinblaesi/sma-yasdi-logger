#include <cassert>
#include <libyasdimaster.h>
#include <iostream>
#include <algorithm>
#include <cstring>
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

Device::~Device() noexcept {
//    RemoveDevice(_handle);
}

void Device::init() {
    bool initSuccess = true;
    constexpr unsigned short maxLength = 64;
    char buffer[maxLength] = {0};
    int rc = GetDeviceName(_handle, buffer, maxLength);
    //assert(rc == YE_OK);
    _name = buffer;
    _name.shrink_to_fit();
    DWORD sn;
    memset(buffer, 0, maxLength);
    rc = GetDeviceSN(_handle, &sn);
//    assert(YE_OK == rc);
    _serialNumber = sn;
    memset(buffer, 0, maxLength);
    rc = GetDeviceType(_handle, buffer, maxLength);
//    assert(YE_OK == rc);
    _type = buffer;
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

void Device::update(DWORD maxAgeSeconds) {
    _channelUpdatesRemaining = _channels.size();
    for (auto &channel : _channels) {
        channel->update(*this, maxAgeSeconds);
    }
}

void Device::onChannelValueReceived(DWORD channelHandle, double value, char *valueText, int errorCode) {
    //assert(isUpdating());
//    cout << "SMA Logger --- Device '" << name() << "': channel value received." << endl;
    auto channel = find_if(_channels.begin(), _channels.end(), [channelHandle](shared_ptr<Channel> const &channel) {
        return channel->handle() == channelHandle;
    });
    if (channel != _channels.end()) {
        (*channel)->onChannelValueReceived(*this, value, valueText, errorCode);
    }
}

/**
 * Called when the async update of a channel is finished
 * @param channel
 * @param channelUpdateResult
 */
void Device::onChannelUpdated(const Channel &channel, ChannelUpdate channelUpdateResult) {
    _channelUpdatesRemaining--;
//    cout << "SMA Logger --- Device '" << name() << "': "
//         << _channelUpdatesRemaining << " channel updates remaining" << endl;
    DeviceUpdate deviceUpdateResult;
    switch (channelUpdateResult) {
        case ChannelUpdate::TIMEOUT:
            if (_online) {
                // mark the device as offline on the first channel timeout event
                // this happens mostly when the inverter is shutting down in the evening
                _online = false;
            }
            deviceUpdateResult = DeviceUpdate::OFFLINE;
            break;
        case ChannelUpdate::FAILURE:
            deviceUpdateResult = DeviceUpdate::FAILURE;
            break;
        case ChannelUpdate::SUCCESS:
            deviceUpdateResult = DeviceUpdate::SUCCESS;
            break;
    }
    // notify master once all channels are updated
    // only the last channel event is presented to the master
    // state of individual channels is kept in the channel class
    if (!isUpdating()) {
        _master.onDeviceUpdated(*this, deviceUpdateResult);
    }
}

const vector<shared_ptr<Channel>> &Device::channels() const {
    return _channels;
}

const bool Device::isUpdating() const {
    return _channelUpdatesRemaining > 0;
}

const bool Device::isOnline() const {
    return _online; // TODO check initialized?
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