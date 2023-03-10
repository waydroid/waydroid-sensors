set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

cmake_minimum_required(VERSION 3.0)
project(waydroid-sensors)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GLIB_UTIL REQUIRED libglibutil)
pkg_check_modules(GBINDER REQUIRED libgbinder>=1.1.20)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")

add_subdirectory(sensorfw-core)

add_executable(
    waydroid-sensord

    SensorFW.cpp
    Sensors.cpp
    service.cpp
)

target_link_libraries(waydroid-sensord PUBLIC
    ${GIO_LDFLAGS} ${GIO_LIBRARIES}
    ${GIO_UNIX_LDFLAGS} ${GIO_UNIX_LIBRARIES}
    ${GLIB_LDFLAGS} ${GLIB_LIBRARIES}
    ${GLIB_UTIL_LDFLAGS} ${GLIB_UTIL_LIBRARIES}
    ${GBINDER_LDFLAGS} ${GBINDER_LIBRARIES}

    sensorfw-core
)

target_include_directories(waydroid-sensord PUBLIC
    ${GIO_INCLUDE_DIRS}
    ${GIO_UNIX_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${GLIB_UTIL_INCLUDE_DIRS}
    ${GBINDER_INCLUDE_DIRS}

    sensorfw-core/include
)

install(TARGETS waydroid-sensord RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
