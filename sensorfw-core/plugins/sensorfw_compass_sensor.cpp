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

#include <plugins/sensorfw_compass_sensor.h>

#include <stdexcept>

namespace
{
auto const null_handler = [](CompassData){};
}

waydroid::core::SensorfwCompassSensor::SensorfwCompassSensor(
    std::string const& dbus_bus_address)
    : Sensorfw(dbus_bus_address, "Compass", PluginType::COMPASS),
      handler{null_handler}
{
}

waydroid::core::HandlerRegistration waydroid::core::SensorfwCompassSensor::register_compass_handler(
    CompassHandler const& handler)
{
    return EventLoopHandlerRegistration{
        dbus_event_loop,
        [this, &handler]{ this->handler = handler; },
        [this]{ this->handler = null_handler; }};
}

void waydroid::core::SensorfwCompassSensor::enable_compass_events()
{
    dbus_event_loop.enqueue(
        [this]
        {
            start();
        }).get();
}

void waydroid::core::SensorfwCompassSensor::disable_compass_events()
{
    dbus_event_loop.enqueue(
        [this]
        {
            stop();
        }).get();
}

void waydroid::core::SensorfwCompassSensor::data_recived_impl()
{
    std::vector<CompassData> values;
    if(!m_socket->read<CompassData>(values))
        return;

    handler(values[0]);
}
