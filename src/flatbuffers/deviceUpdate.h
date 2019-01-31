#pragma once

#include <map>
#include <vector>
#include "../yasdi/Channel.h"
#include "../yasdi/Device.h"

typedef std::map<DWORD, std::vector<yasdi::Channel *>> sortedChannels_t;

namespace device {
    namespace update {
        static inline sortedChannels_t getChannelsByTimestamp(const yasdi::Device &device) {
            sortedChannels_t allChannels;
            for (const auto &channel: device.channels()) {
                if (allChannels.find(channel->timestamp()) == allChannels.end()) {
                    std::vector<yasdi::Channel *> channels;
                    allChannels[channel->timestamp()] = channels;
                } else {
                    allChannels[channel->timestamp()].push_back(channel.get());
                }
            }
            return allChannels;
        }
    }
}