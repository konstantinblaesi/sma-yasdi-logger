#pragma once

#include <libyasdimaster.h>
#include "globals.h"

namespace yasdi {
    namespace callbacks {
        void onDeviceDetected(TYASDIDetectionSub event, DWORD deviceHandle);
        void onChannelValueReceived(DWORD channelHandle, DWORD deviceHandle, double value,
                                               char *valueText, int errorCode);
    }
}