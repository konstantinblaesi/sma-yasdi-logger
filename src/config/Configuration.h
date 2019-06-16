#pragma once

#include <string>

namespace config {
    class Configuration {
    public:
        explicit Configuration(unsigned short expectedDevices,
                               unsigned short restartDetectionIfOnline,
                               unsigned short updateIntervalSeconds,
                               const std::string &yasdiIniFilePath,
                               const std::string &mqttTopicPrefix) noexcept;
        explicit Configuration() noexcept;
        const unsigned short expectedDevices() const;
        const unsigned short restartDetectionIfOnline() const;
        const unsigned short updateIntervalSeconds() const;
        const std::string &yasdiIniFilePath() const;
        const std::string &mqttHost() const;
        const int mqttPort() const;
        const int mqttKeepAliveSeconds() const;
        const std::string &mqttTopicPrefix() const;
        const std::string &mqttUsername() const;
        const std::string &mqttPassword() const;
        void init();
        unsigned short updateValueMaxAgeSeconds() const;

        //

//        std::string _mqttHost;
//        int _mqttPort;
//        std::string _mqttUsername;
//        std::string _mqttPassword;
//        int _mqttKeepAliveSeconds;
//        std::string _mqttTopicPrefix;
    private:
        unsigned short _expectedDevices;
        unsigned short _restartDetectionIfOnline;
        unsigned short _updateValueMaxAgeSeconds;
        unsigned short _updateIntervalSeconds;
        std::string _yasdiIniFilePath;

        std::string _mqttHost;
        int _mqttPort;
        std::string _mqttUsername;
        std::string _mqttPassword;
        int _mqttKeepAliveSeconds;
        std::string _mqttTopicPrefix;
    };
}