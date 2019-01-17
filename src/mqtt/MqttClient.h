#pragma once

#include <mosquitto.h>
#include <string>

#include "../config/Configuration.h"

namespace yasdi {
    class Channel;
    class Device;
}

namespace mqtt {
    enum Qos {
        AtMostOnce = 0,
        AtLeastOnce = 1,
        ExactlyOnce = 2
    };

    class MqttClient {
    public:
        explicit MqttClient(const config::Configuration &config) noexcept;
        ~MqttClient();
        const bool init();
        const bool connect();
        void publish(const yasdi::Device &device) const;
        // @TODO with or without flags?
        void onConnect(int rc, int flags = 0);
        void onDisconnect(int rc);
        void onPublish(int mid);
        void onMessage(const struct mosquitto_message *message);
        void onSubscribe(int mid, int qos_count, const int *granted_qos);
        void onUnsubscribe(int mid);
        void onLog(int level, const char *str);
    private:
        const int publish(int *mid, const std::string &topic, int payloadlen, const void *payload) const;
        bool _initialized;
        bool _connected;
        const config::Configuration _config;
        mosquitto *_mosquittoClient;
    };
};