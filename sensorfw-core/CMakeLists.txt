set(
    SENSORFW_CORE_UTILS_SRCS

    utils/socketreader.cpp
    utils/dbus_connection_handle.cpp
    utils/event_loop.cpp
    utils/handler_registration.cpp

    include/utils/socketreader.h
)

set(
    SENSORFW_CORE_PLUGINS_SRCS

    plugins/sensorfw_accelerometer_sensor.cpp
    plugins/sensorfw_common.cpp
    plugins/sensorfw_compass_sensor.cpp
    plugins/sensorfw_gyroscope_sensor.cpp
    plugins/sensorfw_humidity_sensor.cpp
    plugins/sensorfw_lid_sensor.cpp
    plugins/sensorfw_light_sensor.cpp
    plugins/sensorfw_magnetometer_sensor.cpp
    plugins/sensorfw_orientation_sensor.cpp
    plugins/sensorfw_pressure_sensor.cpp
    plugins/sensorfw_proximity_sensor.cpp
    plugins/sensorfw_rotation_sensor.cpp
    plugins/sensorfw_stepcounter_sensor.cpp
    plugins/sensorfw_tap_sensor.cpp
    plugins/sensorfw_temperature_sensor.cpp
)

add_library(
    sensorfw-core STATIC

    ${SENSORFW_CORE_UTILS_SRCS}
    ${SENSORFW_CORE_PLUGINS_SRCS}
)

target_link_libraries(sensorfw-core PUBLIC
    ${GIO_LDFLAGS} ${GIO_LIBRARIES}
    ${GIO_UNIX_LDFLAGS} ${GIO_UNIX_LIBRARIES}
    ${GLIB_UTIL_LDFLAGS} ${GLIB_UTIL_LIBRARIES}
)

target_include_directories(sensorfw-core PUBLIC
    ${GIO_INCLUDE_DIRS}
    ${GIO_UNIX_INCLUDE_DIRS}
    ${GLIB_UTIL_INCLUDE_DIRS}

    include
)
