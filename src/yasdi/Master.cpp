#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <libyasdimaster.h>
#include <memory>
#include <thread>
#include "../config/Configuration.h"
#include "../mqtt/MqttClient.h"
#include "Master.h"
#include "Channel.h"
#include "Device.h"
#include "Driver.h"

using namespace config;
using namespace std;
using namespace yasdi;

Master::Master(const Configuration &config, const mqtt::MqttClient &mqttClient) noexcept
        : _config(config),
          _mqttClient(mqttClient),
          _initialized(false),
          _detectingDevices(false),
          _deviceUpdatesRemaining(0) {
}

bool Master::init() {
    constexpr size_t maxDrivers = 10;
    DWORD driversCount;
    DWORD driverIDs[maxDrivers];
    int ret = yasdiMasterInitialize(_config.yasdiIniFilePath().c_str(), &driversCount);
    addEventListeners();
    yasdiMasterGetDriver(driverIDs, maxDrivers);
    for (auto i = 0; i < driversCount; i++) {
        unique_ptr<Driver> driver = std::make_unique<Driver>(driverIDs[i]);
        driver->setStatus(true);
        _drivers.push_back(move(driver));
    }
    _initialized = (ret != -1);
    return _initialized;
}

void Master::reset() {
    yasdiReset();
}

bool Master::start() {
    if (init()) {
        startDeviceDetection();
        return true;
    }
    return false;
}

void Master::stop() {
    if (this->isDetectingDevices()) {
        stopDeviceDetection();
    }
    for (auto &driver: _drivers) {
        driver->setStatus(false);
    }
    yasdiMasterShutdown();
}

void Master::addEventListeners() {
    yasdiMasterAddEventListener(reinterpret_cast<void *>(yasdi::callbacks::onDeviceDetected),
                                YASDI_EVENT_DEVICE_DETECTION);
    yasdiMasterAddEventListener(reinterpret_cast<void *>(yasdi::callbacks::onChannelValueReceived),
                                YASDI_EVENT_CHANNEL_NEW_VALUE);
}

void Master::startDeviceDetection() {
    int iErrorCode = DoStartDeviceDetection(_config.expectedDevices(), false);
    switch (iErrorCode) {
        case YE_DEV_DETECT_IN_PROGRESS:
            cout << "SMA Logger --- Master: Cannot detect devices, because detection is in progress."
                 << endl;
            // TODO should never happen?
            _detectingDevices = true;
            break;
        case YE_INVAL_ARGUMENT:
            cout << "SMA Logger --- Master: Cannot detect devices, because the argument is invalid." << endl;
            break;
        case YE_SHUTDOWN:
            cout << "SMA Logger --- Master: Cannot detect devices, because it the master is shut down." << endl;
            break;
        case YE_OK:
            cout << "SMA Logger --- Master: Detecting devices." << endl;
            _detectingDevices = true;
            break;
        default:
            cout << "SMA Logger --- Master: Cannot detect devices, unknown error." << endl;
            // TODO exception?
            break;
    }
}

void Master::stopDeviceDetection() {
//    assert(_detectingDevices);
    cout << "SMA Logger --- " << "Stopping device detection." << endl;
    _detectingDevices = (YE_OK == DoStopDeviceDetection());
//    assert(!_detectingDevices);
}

void Master::onDeviceDetectionFinished() {
    cout << "SMA Logger --- Master: Device detection finished." << endl;
    _detectingDevices = false;
    //updateDevices(_config.updateIntervalSeconds());
}

void Master::onDeviceDetected(TYASDIDetectionSub deviceEvent, DWORD deviceHandle) {
    switch (deviceEvent) {
        case YASDI_EVENT_DEVICE_ADDED: {
            auto device = make_shared<Device>(*this, deviceHandle);
            device->init();
            cout << "SMA Logger --- Master: Discovered device '" << device->name() << "'." << endl;
            _devices.push_back(device);
            if (!isUpdatingDevices()) {
                updateDevices(_config.updateIntervalSeconds());
            }
        }
            break;
        case YASDI_EVENT_DEVICE_REMOVED: {
            // this event doesn't seem to happen when inverters go offline (observation)
            // it's probably triggered by the YASDI SDK method RemoveDevice
            auto removedDevice = find_if(_devices.begin(), _devices.end(),
                                         [deviceHandle](const shared_ptr<Device> &device) {
                                             return device->handle() == deviceHandle;
                                         });
            if (removedDevice != _devices.end()) {
                cout << "SMA Logger --- Master: Removed device '" << (*removedDevice)->name() << "'." << endl;
                _devices.erase(removedDevice);
            }
        }
            break;
        case YASDI_EVENT_DEVICE_SEARCH_END:
            onDeviceDetectionFinished();
            break;
        case YASDI_EVENT_DOWNLOAD_CHANLIST:
            break;
        default:
            cerr << "SMA Logger --- Master: Unknown YASDI device event." << endl;
            break;
    }
}

void Master::onChannelValueReceived(DWORD channelHandle, DWORD deviceHandle, double valueNumber,
                                    char *valueText, int errorCode) {
//    assert(_initialized);
    //assert(this->isUpdatingDevices());
    auto device = find_if(_devices.begin(), _devices.end(), [deviceHandle](const shared_ptr<Device> &device) {
        return deviceHandle == device->handle();
    });
//    assert(device != _devices.end());
    (*device)->onChannelValueReceived(channelHandle, valueNumber, valueText, errorCode);
}

void Master::onDeviceUpdated(const Device &device) {
//    assert(_initialized);
    _deviceUpdatesRemaining--;
    // doesn't make sense for last updated event
    //assert(this->isUpdatingDevices());
    _mqttClient.publish(device);
    if (!this->isUpdatingDevices()) {
        this->onDeviceUpdatesFinished();
    }
}

void Master::onDeviceUpdatesFinished() {
    _updateEnd = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(_updateEnd - _updateBegin).count();
    cout << "SMA Logger --- Master: Updating devices finished after "
         << duration << " milliseconds." << endl;
    auto sleep = max(static_cast<long>((_config.updateIntervalSeconds() * 1000) - duration), static_cast<long>(0));
    cout << "SMA Logger --- Master: Waiting "
         << sleep << " milliseconds until next update." << endl;
    this_thread::sleep_for(chrono::milliseconds(sleep));
    this->removeOfflineDevices();
    updateDevices(_config.updateIntervalSeconds());
}

void Master::removeOfflineDevices() {
    auto removed = _devices.erase(remove_if(_devices.begin(), _devices.end(), [&](const shared_ptr<Device> device) {
        return !device->isOnline();
    }), _devices.end());
    // assert(removed != _devices.end());
}

void Master::onDeviceOffline(const Device &device) {
    cout << "SMA Logger --- "
         << "Master '" << device.name() << "' went offline.";
}

void Master::updateDevices(DWORD maxAgeSeconds) {
//    assert(!isUpdatingDevices());
    if (_devices.empty()) {
        return;
    }
    cout << "SMA Logger --- Master: Updating " << _devices.size()
         << " devices requesting a max channel value age of " << maxAgeSeconds
         << " seconds." << endl;
    _updateBegin = std::chrono::high_resolution_clock::now();
    _deviceUpdatesRemaining = _devices.size();
    for (const auto &device: _devices) {
//        assert(device->isOnline());
        device->updateChannels(maxAgeSeconds);
    }
}

bool Master::isUpdatingDevices() const {
    return _deviceUpdatesRemaining > 0;
}

bool Master::isDetectingDevices() const {
    return _detectingDevices;
}

bool Master::isInitialized() const {
    return _initialized;
}