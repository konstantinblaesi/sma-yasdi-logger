#pragma once

#include <memory>
#include <string>
#include <smadef.h>
#include <vector>

#include "Channel.h"

namespace mqtt {
    class MqttClient;
}

namespace yasdi {
    class Master;

    class Channel;

    enum class DeviceUpdate {
        SUCCESS,
        FAILURE,
        OFFLINE
    };

    class Device {
    public:
        explicit Device(yasdi::Master &master, DWORD deviceHandle) noexcept;
        Device() = delete;
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        ~Device() noexcept;
        void init();
        void update(DWORD maxAgeSeconds);
        void onChannelValueReceived(DWORD channelHandle, double value,
                                    char *valueText, int errorCode);
        void onChannelUpdated(const Channel &channel, const ChannelUpdate channelUpdateResult);
        const bool isOnline() const;
        const DWORD handle() const;
        const std::string name() const;
        const std::string type() const;
        const DWORD serialNumber() const;
        const bool isUpdating() const;
        const std::vector<std::shared_ptr<Channel>> &channels() const;
    private:
        yasdi::Master &_master;
        DWORD _handle;
        std::string _name;
        std::string _type;
        DWORD _serialNumber;
        bool _online;
        bool _initialized;
        std::vector<std::shared_ptr<Channel>> _channels;
        size_t _channelUpdatesRemaining;
    };
}