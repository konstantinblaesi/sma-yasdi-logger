#include <memory>

#include <csignal>
#include <iostream>
#include <functional>
#include <thread>

#include <libyasdimaster.h>
#include <memory>

#include "mqtt/MqttClient.h"
#include "globals.h"
#include "yasdi/Master.h"

yasdi::Master *yMaster;
bool keepRunning = true;

using namespace config;
using namespace yasdi;
using namespace std;
using namespace mqtt;

void handleError(bool failed, const string &msg) {
    if (failed) {
        cerr << "SMA Logger --- " << msg << ": Failure." << endl;
        exit(1);
    } else {
        cerr << "SMA Logger --- " << msg << ": Success." << endl;
    }
}

void onSignal(int signal) {
    cout << "SMA Logger --- " << "Received sigal " << signal << "." << endl;
    cout << "SMA Logger --- " << "Shutting down." << endl;
    yMaster->stop();
    keepRunning = false;
}

int main(int argc, char **argv) {
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    cout << "SMA Logger --- " << "Data logger for SMA inverters starting." << endl;
    Configuration config;
    config.init();

    cout << "SMA Logger --- Configuration: Expected devices '" << config.expectedDevices() << "'." << endl;
    cout << "SMA Logger --- Configuration: Restart device detection if online devices count '" << config.restartDetectionIfOnline() << "'." << endl;
    cout << "SMA Logger --- Configuration: Channel update interval in seconds '" << config.updateIntervalSeconds() << "'." << endl;
    cout << "SMA Logger --- Configuration: YASDI ini path '" << config.yasdiIniFilePath() << "'." << endl;

    cout << "SMA Logger --- Configuration: MQTT host '" << config.mqttHost() << "'." << endl;
    cout << "SMA Logger --- Configuration: MQTT port '" << config.mqttPort() << "'." << endl;
    cout << "SMA Logger --- Configuration: MQTT username '" << config.mqttUsername() << "'." << endl;
    cout << "SMA Logger --- Configuration: MQTT password '" << config.mqttPassword() << "'." << endl;
    cout << "SMA Logger --- Configuration: MQTT keep alive seconds '" << config.mqttKeepAliveSeconds() << "'." << endl;
    cout << "SMA Logger --- Configuration: MQTT topic prefix '" << config.mqttTopicPrefix() << "'." << endl;
    MqttClient mqttClient(config);
    handleError(!mqttClient.init(), "MQTT client init");
    handleError(!mqttClient.connect(), "MQTT client connect");
    Master master(config, mqttClient);
    yMaster = &master;
    handleError(!master.start(), "YASDI Master start");
    while (keepRunning) {
        this_thread::sleep_for(chrono::seconds(1));
    }
    return 0;
}

