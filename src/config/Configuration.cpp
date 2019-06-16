#include "Configuration.h"

using namespace config;
using namespace std;

Configuration::Configuration(unsigned short expectedDevices,
                             unsigned short restartDetectionIfOnline,
                             unsigned short updateIntervalSeconds,
                             const string &yasdiIniFilePath,
                             const string &mqttTopicPrefix) noexcept:
        _expectedDevices(expectedDevices),
        _restartDetectionIfOnline(restartDetectionIfOnline),
        _updateIntervalSeconds(updateIntervalSeconds),
        _yasdiIniFilePath(yasdiIniFilePath),
        _mqttTopicPrefix(mqttTopicPrefix) {
}

Configuration::Configuration() noexcept {
}

void Configuration::init() {
    _expectedDevices = static_cast<unsigned short>(stoi(getenv("EXPECTED_DEVICES")));
    _restartDetectionIfOnline = static_cast<unsigned short>(stoi(getenv("RESTART_DETECTION_IF_ONLINE")));
    _updateValueMaxAgeSeconds = static_cast<unsigned short>(stoi(getenv("UPDATE_VALUE_MAX_AGE_SECONDS")));
    _updateIntervalSeconds = static_cast<unsigned short>(stoi(getenv("UPDATE_INTERVAL_SECONDS")));
    _yasdiIniFilePath = getenv("YASDI_INI_FILE_PATH");

    _mqttHost = getenv("MQTT_HOST");
    _mqttPort = static_cast<unsigned short>(stoi(getenv("MQTT_PORT")));
    _mqttUsername = getenv("MQTT_USERNAME");
    _mqttPassword = getenv("MQTT_PASSWORD");
    _mqttKeepAliveSeconds = static_cast<unsigned short>(stoi(getenv("MQTT_KEEP_ALIVE_SECONDS")));
    _mqttTopicPrefix = getenv("MQTT_TOPIC_PREFIX");
}

const unsigned short Configuration::expectedDevices() const {
    return _expectedDevices;
}

const unsigned short Configuration::restartDetectionIfOnline() const {
    return _restartDetectionIfOnline;
}

unsigned short Configuration::updateValueMaxAgeSeconds() const {
    return _updateValueMaxAgeSeconds;
}

const unsigned short Configuration::updateIntervalSeconds() const {
    return _updateIntervalSeconds;
}

const string &Configuration::yasdiIniFilePath() const {
    return _yasdiIniFilePath;
}

const string &Configuration::mqttHost() const {
    return _mqttHost;
}

const int Configuration::mqttPort() const {
    return _mqttPort;
}

const string &Configuration::mqttUsername() const {
    return _mqttUsername;
}

const string &Configuration::mqttPassword() const {
    return _mqttPassword;
}

const int Configuration::mqttKeepAliveSeconds() const {
    return _mqttKeepAliveSeconds;
}

const string &Configuration::mqttTopicPrefix() const {
    return _mqttTopicPrefix;
}
