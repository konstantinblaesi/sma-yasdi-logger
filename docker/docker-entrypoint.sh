#!/bin/sh
set -e

source /etc/sma_logger/logger.env

export EXPECTED_DEVICES
export RESTART_DETECTION_IF_ONLINE
export UPDATE_INTERVAL_SECONDS
export YASDI_INI_FILE_PATH

export MQTT_HOST
export MQTT_PORT
export MQTT_USERNAME
export MQTT_PASSWORD
export MQTT_KEEP_ALIVE_SECONDS
export MQTT_TOPIC_PREFIX

exec $@