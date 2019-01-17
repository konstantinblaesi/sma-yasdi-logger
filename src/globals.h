#pragma once

namespace yasdi {
    class Master;
}

//namespace yasdi::global {
//    // needed because non-static members cannot be used as callbacks with async yasdi APIs
    extern yasdi::Master* yMaster;
//}
