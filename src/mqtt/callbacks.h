#pragma once

#include "MqttClient.h"

// these wrappers forward moquitto events to our MqttClient instance
namespace mqtt {
    namespace callbacks {
        static void onConnect(struct mosquitto *mosq, void *userdata, int rc) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onConnect(rc);
        }

        static void onConnectWithFlags(struct mosquitto *mosq, void *userdata, int rc, int flags) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onConnect(rc, flags);
        }

        static void onDisconnect(struct mosquitto *mosq, void *userdata, int rc) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onDisconnect(rc);
        }

        static void onPublish(struct mosquitto *mosq, void *userdata, int mid) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onPublish(mid);
        }

        static void onMessage(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onMessage(message);
        }

        static void
        onSubscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onSubscribe(mid, qos_count, granted_qos);
        }

        static void onUnsubscribe(struct mosquitto *mosq, void *userdata, int mid) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onUnsubscribe(mid);
        }


        static void onLog(struct mosquitto *mosq, void *userdata, int level, const char *str) {
            MqttClient *mqttClient = reinterpret_cast<MqttClient *>(userdata);
            mqttClient->onLog(level, str);
        }
    }
}