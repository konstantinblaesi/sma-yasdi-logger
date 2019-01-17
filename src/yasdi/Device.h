#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <smadef.h>

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
        bool isOnline() const;
        bool setStatus(bool isOnline);
        DWORD handle() const;
        std::string name() const;
        std::string type() const;
        void updateChannels(DWORD maxAgeSeconds);
        DWORD serialNumber() const;
        bool isUpdating() const;
        std::vector<std::shared_ptr<Channel>> channels() const;
        void onChannelTimeout(const Channel &channel);
        void onChannelValueReceived(DWORD channelHandle, double value,
                                    char *valueText, int errorCode);
        void onChannelUpdated(const Channel &channel);
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
        std::mutex _mutex;
    };
}