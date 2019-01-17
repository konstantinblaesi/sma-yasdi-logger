#include <iostream>
#include "delegates.h"
#include "globals.h"
#include "yasdi/Master.h"

namespace yasdi {
    namespace callbacks {
        void onDeviceDetected(TYASDIDetectionSub event, DWORD deviceHandle) {
            yMaster->onDeviceDetected(event, deviceHandle);
        }
        void onChannelValueReceived(DWORD channelHandle, DWORD deviceHandle, double value,
                                       char *valueText, int errorCode) {
            yMaster->onChannelValueReceived(channelHandle, deviceHandle, value, valueText, errorCode);
        }
    }
}
