#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <string>
#include <smadef.h>
#include <atomic>

namespace mqtt {
    class MqttClient;
}

namespace yasdi {
    class Master;

    class Channel;

    class Device {
    public:
        explicit Device(yasdi::Master &master, DWORD deviceHandle) noexcept;
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        ~Device() = default;
        void init();
        void updateChannels(DWORD maxAgeSeconds);
        void onChannelTimeout(const Channel &channel);
        void onChannelValueReceived(DWORD channelHandle, double value,
                                    char *valueText, int errorCode);
        void onChannelUpdated(const Channel &channel);
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
        std::atomic_bool _online;
        bool _initialized;
        std::vector<std::shared_ptr<Channel>> _channels;
        std::atomic_size_t _channelUpdatesRemaining;
        mutable std::shared_mutex _mutex;
    };
}