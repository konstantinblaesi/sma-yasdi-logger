#pragma once

#include <mutex>
#include <smadef.h>
#include <string>

namespace yasdi {
    class Master;
    class Device;

    class Channel {
    public:
        explicit Channel(yasdi::Device &device, DWORD handle) noexcept;
        Channel(const Channel &) = delete;
        Channel &operator=(const Channel &) = delete;
        ~Channel() = default;
        bool update(DWORD maxAgeSeconds);
        double value() const;
        std::string valueText() const;
        bool isUpdating() const;
        bool isValid() const;
        const std::string &name() const;
        DWORD handle() const;
        std::string unit() const;
        DWORD timestamp() const;
        void onChannelValueReceived(Device &device, double value, char *valueText, int errorCode);
    private:
        DWORD _handle;
        double _value;
        std::string _valueText;
        std::string _name;
        std::string _unit;
        DWORD _timestamp;
        bool _updating;
        bool _valueValid;
        Device &_device;
        std::mutex _mutex;
    };
}