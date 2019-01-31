#pragma once

#include <smadef.h>
#include <string>

namespace yasdi {
    class Driver {
    public:
        explicit Driver(DWORD id) noexcept;
        Driver(const Driver&) = delete;
        Driver& operator=(const Driver&) = delete;
        ~Driver() noexcept;
        const DWORD id() const;
        const bool isOnline() const;
    private:
        DWORD _id;
        std::string _name;
        bool _online;
    };
}