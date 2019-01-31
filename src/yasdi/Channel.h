#pragma once

#include <smadef.h>
#include <string>

namespace yasdi {
    class Master;

    class Device;

    enum class ChannelUpdate {
        SUCCESS,
        TIMEOUT,
        FAILURE
    };

    class Channel {
    public:
        explicit Channel(DWORD handle) noexcept;
        Channel() = delete;
        Channel(const Channel &) = delete;
        Channel &operator=(const Channel &) = delete;
        ~Channel() = default;
        const bool update(Device &device, DWORD maxAgeSeconds);
        const double value() const;
        const std::string valueText() const;
        const bool isUpdating() const;
        const bool isValid() const;
        const bool isNumeric() const;
        const std::string &name() const;
        const DWORD handle() const;
        const std::string unit() const;
        const DWORD timestamp() const;
        void onChannelValueReceived(Device &device, const double value, const char *valueText, const int errorCode);
    private:
        DWORD _handle;
        double _value;
        std::string _valueText;
        std::string _name;
        std::string _unit;
        DWORD _timestamp;
        bool _updating;
        bool _valueValid;
    };
}