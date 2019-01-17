#include <iostream>
#include <sstream>
#include "../yasdi/Channel.h"
#include "../yasdi/Device.h"
#include "MqttClient.h"
#include "callbacks.h"
#include "../config/Configuration.h"
#include "../../flatbuffers/include/device_update_generated.h"

using namespace config;
using namespace mqtt;
using namespace std;
using namespace yasdi;
using namespace flatbuffers;
using namespace logger::flatbuffer;

MqttClient::MqttClient(const Configuration &config) noexcept :
        _config(config),
        _initialized(false),
        _connected(false) {
}

MqttClient::~MqttClient() {
    mosquitto_loop_stop(_mosquittoClient, false);
    mosquitto_destroy(_mosquittoClient);
    mosquitto_lib_cleanup();
};

const bool MqttClient::init() {
    bool initSuccess = true;
    mosquitto_lib_init();
    _mosquittoClient = mosquitto_new(nullptr, true, this);
    if (!_mosquittoClient) {
        initSuccess = false;
        switch (errno) {
            case ENOMEM:
                break;
            case EINVAL:
                break;
            default:
                break;
        }
        mosquitto_username_pw_set(_mosquittoClient, _config.mqttUsername().c_str(), _config.mqttPassword().c_str());
        //mosquitto_connect_callback_set(_mosquittoClient, callbacks::onConnect);
        mosquitto_connect_with_flags_callback_set(_mosquittoClient, callbacks::onConnectWithFlags);
        mosquitto_disconnect_callback_set(_mosquittoClient, callbacks::onDisconnect);
        mosquitto_publish_callback_set(_mosquittoClient, callbacks::onPublish);
        mosquitto_message_callback_set(_mosquittoClient, callbacks::onMessage);
        mosquitto_subscribe_callback_set(_mosquittoClient, callbacks::onSubscribe);
        mosquitto_unsubscribe_callback_set(_mosquittoClient, callbacks::onUnsubscribe);
        mosquitto_log_callback_set(_mosquittoClient, callbacks::onLog);
        mosquitto_loop_start(_mosquittoClient);
    }
    _initialized = initSuccess;
    return _initialized;
}

const bool MqttClient::connect() {
    assert(_initialized);

    cout << "SMA Logger --- MqttClient: trying to connect, waiting ..." << endl;
    bool success = (MOSQ_ERR_SUCCESS == mosquitto_connect_async(
            _mosquittoClient,
            _config.mqttHost().c_str(),
            _config.mqttPort(),
            _config.mqttKeepAliveSeconds()
    ));
    success = success && (MOSQ_ERR_SUCCESS == mosquitto_loop_start(_mosquittoClient));
    _connected = success;
    return _connected;
}

void MqttClient::publish(const Device &device) const {
    if (_connected) {
        FlatBufferBuilder fbb;
        vector<Offset<SpotChannel>> spotChannels;
        for (const auto &channel: device.channels()) {
            spotChannels.push_back(CreateSpotChannelDirect(
                    fbb,
                    channel->name().c_str(),
                    channel->value(),
                    channel->valueText().c_str(),
                    channel->unit().c_str(),
                    channel->timestamp()
            ));
        }
        auto deviceUpdate = CreateDeviceUpdate(
                fbb,
                fbb.CreateString(device.name()),
                fbb.CreateString(device.type()),
                device.serialNumber(),
                fbb.CreateVector(spotChannels)
        );
        fbb.Finish(deviceUpdate);
        stringstream topic;
        topic << _config.mqttTopicPrefix() << "/Devices/" << device.serialNumber();
        // @TODO 2nd parameter: optionally pass pointer to int to track onChannelUpdated status via onPublish callback
        publish(nullptr, topic.str(), fbb.GetSize(), fbb.GetBufferPointer());
    }
}

const int MqttClient::publish(int *mid, const string &topic, int payloadlen, const void *payload) const {
    return mosquitto_publish(_mosquittoClient, mid, topic.c_str(), payloadlen, payload, AtMostOnce, false);
}

void MqttClient::onConnect(int rc, int flags) {
    switch (rc) {
        case MOSQ_ERR_SUCCESS:
            _connected = true;
            cout << "SMA Logger --- MqttClient: connected successfully" << endl;
            break;
        case MOSQ_ERR_INVAL:
        case MOSQ_ERR_ERRNO:
        default:
            cout << "SMA Logger --- MqttClient: connection error" << endl;
            _connected = false;
            break;
    }
}

void MqttClient::onDisconnect(int rc) {
    cerr << "SMA Logger --- MqttClient: disconnected" << endl;
}

void MqttClient::onPublish(int mid) {
    //cout << "SMA Logger --- MqttClient: Published message with ID: " << mid << endl;
}

void MqttClient::onMessage(const struct mosquitto_message *message) {

}

void MqttClient::onSubscribe(int mid, int qos_count, const int *granted_qos) {

}

void MqttClient::onUnsubscribe(int mid) {

}

void MqttClient::onLog(int level, const char *str) {
    string prefix = "SMA Logger --- MqttClient:";
    switch (level) {
        case MOSQ_LOG_INFO:
            cout << prefix << " INFO: " << str << endl;
            break;
        case MOSQ_LOG_NOTICE:
            cout << prefix << " NOTICE: " << str << endl;
            break;
        case MOSQ_LOG_WARNING:
            cout << prefix << " WARNING: " << str << endl;
            break;
        case MOSQ_LOG_ERR:
            cerr << prefix << " ERROR: " << str << endl;
            break;
        case MOSQ_LOG_DEBUG:
            break;
    }
}
