#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <libyasdimaster.h>
#include "Driver.h"
#include "../delegates.h"

namespace config {
    class Configuration;
}

namespace mqtt {
    class MqttClient;
}

namespace yasdi {
    class Channel;
    class Device;

    class Master {
    public:
        explicit Master(const config::Configuration &config, const mqtt::MqttClient &mqttClient) noexcept;
        Master(const Master &) = delete;
        Master &operator=(const Master &) = delete;
        ~Master() = default;
        bool start();
        void stop();
        void reset();
        void startDeviceDetection();
        void stopDeviceDetection();
        void updateDevices(DWORD maxAgeSeconds);
        bool isUpdatingDevices() const;
        bool isDetectingDevices() const;
        bool isInitialized() const;
        void publishChannel(const Device &device, const yasdi::Channel& channel);
        void onDeviceDetected(TYASDIDetectionSub deviceEvent, DWORD deviceHandle);
        void onChannelValueReceived(DWORD channelHandle, DWORD deviceHandle, double value,
                                    char *valueText, int errorCode);
        void onDeviceUpdated(const Device &device);
        void onDeviceOffline(const Device &device);
    private:
        bool init();
        void addEventListeners();
        void onDeviceDetectionFinished();
        const config::Configuration &_config;
        bool _initialized;
        bool _detectingDevices;
        std::vector<std::unique_ptr<Driver>> _drivers;
        std::vector<std::shared_ptr<Device>> _devices;
        size_t _deviceUpdatesRemaining;
        std::chrono::high_resolution_clock::time_point _updateBegin;
        std::chrono::high_resolution_clock::time_point _updateEnd;
        const mqtt::MqttClient &_mqttClient;
        std::mutex _mutex;
    };
}
