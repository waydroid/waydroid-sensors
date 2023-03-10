/*
 * Copyright © 2020 UBports foundation
 * Copyright © 2021 Waydroid Project.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Marius Gripsgard <marius@ubports.com>
 * Authored by: Erfan Abdi <erfangplus@gmail.com>
 */

#include <memory>
#include <string>
#include <thread>

#include <utils/dbus_connection_handle.h>
#include <utils/event_loop_handler_registration.h>
#include <utils/socketreader.h>

#include <gutil_log.h>

#pragma once

class SocketReader;
namespace waydroid {
namespace core {
class Sensorfw {
public:
    enum PluginType
    {
        ACCELEROMETER,
        COMPASS,
        GYROSCOPE,
        HUMIDITY,
        LID,
        LIGHT,
        MAGNETOMETER,
        ORIENTATION,
        PRESSURE,
        PROXIMITY,
        ROTATION,
        STEPCOUNTER,
        TAP,
        TEMPERATURE
    };

    Sensorfw(
        std::string const& dbus_bus_address,
        std::string const& name,
        PluginType const& plugin);
    virtual ~Sensorfw();

protected:
    virtual void data_recived_impl() = 0;

    void set_interval(int interval = 10);
    void start();
    void stop();

    DBusConnectionHandle dbus_connection;
    EventLoop dbus_event_loop;
    std::shared_ptr<SocketReader> m_socket;

private:
    void request_sensor();
    bool release_sensor();
    bool load_plugin();

    const char* plugin_string() const;
    const char* plugin_interface() const;
    const char* plugin_path() const;

    static gboolean static_data_recieved(GSocket * socket, GIOCondition cond, gpointer user_data);

    PluginType m_plugin;
    std::unique_ptr<char, decltype(&free)> m_pluginPath;
    pid_t m_pid;
    int m_sessionid;
    std::unique_ptr<GSource, decltype(&g_source_unref)> m_gsource;
};
}
}
