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

#include <plugins/sensorfw_light_sensor.h>

#include <stdexcept>

namespace
{
auto const null_handler = [](TimedUnsigned){};
}

waydroid::core::SensorfwLightSensor::SensorfwLightSensor(
    std::string const& dbus_bus_address)
    : Sensorfw(dbus_bus_address, "Light", PluginType::LIGHT),
      handler{null_handler}
{
}

waydroid::core::HandlerRegistration waydroid::core::SensorfwLightSensor::register_light_handler(
    LightHandler const& handler)
{
    return EventLoopHandlerRegistration{
        dbus_event_loop,
        [this, &handler]{ this->handler = handler; },
        [this]{ this->handler = null_handler; }};
}

void waydroid::core::SensorfwLightSensor::enable_light_events()
{
    dbus_event_loop.enqueue(
        [this]
        {
            start();
        }).get();
}

void waydroid::core::SensorfwLightSensor::disable_light_events()
{
    dbus_event_loop.enqueue(
        [this]
        {
            stop();
        }).get();
}

void waydroid::core::SensorfwLightSensor::data_recived_impl()
{
    std::vector<TimedUnsigned> values;
    if(!m_socket->read<TimedUnsigned>(values))
        return;

    handler(values[0]);
}
