cmake_minimum_required(VERSION 2.8)
project(sma_logger)

set(CMAKE_CXX_STANDARD 20)

if (NOT DEFINED YASDI_SRC)
    get_filename_component(YASDI_SRC "../yasdi" ABSOLUTE)
endif ()

include_directories(
        ${YASDI_SRC}/include
        ${YASDI_SRC}/core
        ${YASDI_SRC}/smalib
        ${YASDI_SRC}/os
        ${YASDI_SRC}/protocol
        ${YASDI_SRC}/master
        ${YASDI_SRC}/libs
        ${YASDI_SRC}/projects/generic-cmake/incprj
)
link_directories(${YASDI_SRC}/projects/generic-cmake/build)

add_executable(sma_logger src/main.cpp src/yasdi/Master.cpp src/yasdi/Master.h src/yasdi/Device.cpp
        src/yasdi/Device.h src/globals.h src/delegates.h src/delegates.cpp src/yasdi/Driver.cpp src/yasdi/Driver.h
        src/yasdi/Channel.cpp src/yasdi/Channel.h src/config/Configuration.h src/config/Configuration.cpp src/utils.h src/mqtt/MqttClient.cpp src/mqtt/MqttClient.h src/mqtt/callbacks.h src/flatbuffers/deviceUpdate.h)
target_link_libraries(sma_logger pthread yasdi yasdimaster mosquitto)

install(TARGETS sma_logger
        RUNTIME DESTINATION bin)
