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
    for (auto &driverID: driverIDs) {
        _drivers.push_back(std::make_unique<Driver>(driverID));
    }
    _initialized = (ret != -1);
    return _initialized;
}

void Master::reset() {
    yasdiReset();
}

bool Master::start() {
    if (!init()) {
        return false;
    }
    startDeviceDetection();
    return true;
}

void Master::stop() {
    if (this->isDetectingDevices()) {
        stopDeviceDetection();
    }
    _drivers.clear();
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
                // start update loop once as soon as the first device is online
                updateDevices(_config.updateIntervalSeconds());
            }
        }
            break;
        case YASDI_EVENT_DEVICE_REMOVED:
            // triggered by YASDI SDK method RemoveDevice
            break;
        case YASDI_EVENT_DEVICE_SEARCH_END:
            onDeviceDetectionFinished();
            break;
        case YASDI_EVENT_DOWNLOAD_CHANLIST:
            // downloading channel list of device
            break;
        default:
            //cerr << "SMA Logger --- Master: Unknown YASDI device event." << endl;
            break;
    }
}

/**
 * Called by the YASDI SDK
 * this method forwards a single asynchronously received channel value
 * to the device it belongs to
 * @param channelHandle
 * @param deviceHandle
 * @param valueNumber
 * @param valueText
 * @param errorCode
 */
void Master::onChannelValueReceived(DWORD channelHandle, DWORD deviceHandle, double valueNumber,
                                    char *valueText, int errorCode) {
    auto device = find_if(_devices.begin(), _devices.end(), [deviceHandle](const shared_ptr<Device> &device) {
        return deviceHandle == device->handle();
    });
    if (device != _devices.end()) {
        (*device)->onChannelValueReceived(channelHandle, valueNumber, valueText, errorCode);
    }
}

/**
 * Called when the async update of the devices' channels is done
 * @param device
 * @param updateResult
 */
void Master::onDeviceUpdated(const Device &device, DeviceUpdate updateResult) {
    _deviceUpdatesRemaining--;
    switch (updateResult) {
        case DeviceUpdate::FAILURE:
            break;
        case DeviceUpdate::OFFLINE:
            cout << "SMA Logger --- "
                 << "Master: Device '" << device.name() << "' went offline." << endl;
            break;
        case DeviceUpdate::SUCCESS:
            _mqttClient.publish(device);
            break;
    }
    cout << "SMA Logger --- Master: " << _deviceUpdatesRemaining << " device updates remaining" << endl;
    if (!this->isUpdatingDevices()) {
        this->onDeviceUpdatesFinished();
    }
}

void Master::onDeviceUpdatesFinished() {
    _updateEnd = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(_updateEnd - _updateBegin).count();
    cout << "SMA Logger --- Master: Updating devices finished after "
         << duration << " milliseconds." << endl;
    auto delay = max(static_cast<long>((_config.updateIntervalSeconds() * 1000) - duration), static_cast<long>(0));
    cout << "SMA Logger --- Master: Waiting "
         << delay << " milliseconds until next update." << endl;
    this_thread::sleep_for(chrono::milliseconds(delay));
    this->removeOfflineDevices();
    updateDevices(_config.updateIntervalSeconds());
}

void Master::removeOfflineDevices() {
    size_t sizeBefore = _devices.size();
    _devices.erase(remove_if(_devices.begin(), _devices.end(), [](const shared_ptr<Device> &device) {
        return !device->isOnline();
    }), _devices.end());
    auto devicesRemoved = sizeBefore - _devices.size();
    if (devicesRemoved > 0) {
        cout << "SMA Logger --- Master: " << devicesRemoved << " offline devices were removed." << endl;
        cout << "SMA Logger --- Master: " << _devices.size() << " online devices are remaining." << endl;
    }
}

void Master::updateDevices(DWORD maxAgeSeconds) {
    if (_devices.empty()) {
        // all devices offline, start detecting
        startDeviceDetection();
        return;
    }
    cout << "SMA Logger --- Master: Updating " << _devices.size()
         << " devices requesting a max channel value age of " << maxAgeSeconds
         << " seconds." << endl;
    _updateBegin = std::chrono::high_resolution_clock::now();
    _deviceUpdatesRemaining = _devices.size();
    for (const auto &device: _devices) {
        device->update(maxAgeSeconds);
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