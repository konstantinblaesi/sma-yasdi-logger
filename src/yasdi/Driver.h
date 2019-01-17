#pragma once

#include <mutex>
#include <smadef.h>
#include <string>

namespace yasdi {
    class Driver {
    public:
        explicit Driver(DWORD id) noexcept;
        Driver(const Driver&) = delete;
        Driver& operator=(const Driver&) = delete;
        ~Driver() {};
        const DWORD id() const;
        const bool setStatus(bool value);
        const bool isOnline() const;
    private:
        DWORD _id;
        std::string _name;
        bool _online;
        std::mutex _mutex;
    };
}