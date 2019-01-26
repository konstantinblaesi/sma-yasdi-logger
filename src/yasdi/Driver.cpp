#include <libyasdimaster.h>
#include "Driver.h"

using namespace yasdi;

Driver::Driver(DWORD id) noexcept
        : _id(id),
          _online(false) {
}

const DWORD Driver::id() const {
    return _id;
}

const bool Driver::isOnline() const {
    return _online;
}

const bool Driver::setStatus(bool value) {
    if (value) {
        _online = (yasdiMasterSetDriverOnline(_id) == TRUE);
    } else {
        yasdiMasterSetDriverOffline(_id);
        _online = false;
    }
    return this->isOnline();
}
