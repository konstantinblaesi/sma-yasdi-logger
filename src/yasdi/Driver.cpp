#include <libyasdimaster.h>
#include "Driver.h"

using namespace yasdi;

Driver::Driver(DWORD id) noexcept
        : _id(id),
          _online(false) {
    _online = (yasdiMasterSetDriverOnline(_id) == TRUE);
}

Driver::~Driver() noexcept {
    if (_online) {
        yasdiMasterSetDriverOffline(_id);
    }
}

const DWORD Driver::id() const {
    return _id;
}

const bool Driver::isOnline() const {
    return _online;
}