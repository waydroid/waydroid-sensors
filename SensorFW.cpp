/*
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
 * Authored by: Erfan Abdi <erfangplus@gmail.com>
 */

#include "SensorFW.h"
#include <gio/gio.h>
#include <iostream>

namespace waydroid {

std::string the_dbus_bus_address()
{
	auto const address = std::unique_ptr<gchar, decltype(&g_free)>{
		g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr),
		g_free};

	return address ? address.get() : std::string{};
}

SensorFW::SensorFW()
    : data(nullptr) {
    std::string dbus_address = the_dbus_bus_address();
    data = g_new0(SensorData, 1);

    try {
        data->accelerometer_sensor = std::make_shared<waydroid::core::SensorfwAccelerometerSensor>(dbus_address);
        data->sensorAvailable[ID_ACCELEROMETER] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwAccelerometerSensor: %s", e.what());
        data->sensorAvailable[ID_ACCELEROMETER] = FALSE;
    }
    try {
        data->gyroscope_sensor = std::make_shared<waydroid::core::SensorfwGyroscopeSensor>(dbus_address);
        data->sensorAvailable[ID_GYROSCOPE] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwGyroscopeSensor: %s", e.what());
        data->sensorAvailable[ID_GYROSCOPE] = FALSE;
    }
    try {
        data->humidity_sensor = std::make_shared<waydroid::core::SensorfwHumiditySensor>(dbus_address);
        data->sensorAvailable[ID_HUMIDITY] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwHumiditySensor: %s", e.what());
        data->sensorAvailable[ID_HUMIDITY] = FALSE;
    }
    try {
        data->light_sensor = std::make_shared<waydroid::core::SensorfwLightSensor>(dbus_address);
        data->sensorAvailable[ID_LIGHT] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwLightSensor: %s", e.what());
        data->sensorAvailable[ID_LIGHT] = FALSE;
    }
    try {
        data->magnetometer_sensor = std::make_shared<waydroid::core::SensorfwMagnetometerSensor>(dbus_address);
        data->sensorAvailable[ID_MAGNETIC_FIELD] = TRUE;
        data->sensorAvailable[ID_MAGNETIC_FIELD_UNCALIBRATED] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwMagnetometerSensor: %s", e.what());
        data->sensorAvailable[ID_MAGNETIC_FIELD] = FALSE;
        data->sensorAvailable[ID_MAGNETIC_FIELD_UNCALIBRATED] = FALSE;
    }
    try {
        data->orientation_sensor = std::make_shared<waydroid::core::SensorfwOrientationSensor>(dbus_address);
        data->sensorAvailable[ID_DEVICE_ORIENTATION] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwOrientationSensor: %s", e.what());
        data->sensorAvailable[ID_DEVICE_ORIENTATION] = FALSE;
    }
    try {
        data->pressure_sensor = std::make_shared<waydroid::core::SensorfwPressureSensor>(dbus_address);
        data->sensorAvailable[ID_PRESSURE] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwPressureSensor: %s", e.what());
        data->sensorAvailable[ID_PRESSURE] = FALSE;
    }
    try {
        data->proximity_sensor = std::make_shared<waydroid::core::SensorfwProximitySensor>(dbus_address);
        data->sensorAvailable[ID_PROXIMITY] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwProximitySensor: %s", e.what());
        data->sensorAvailable[ID_PROXIMITY] = FALSE;
    }
    try {
        data->stepcounter_sensor = std::make_shared<waydroid::core::SensorfwStepcounterSensor>(dbus_address);
        data->sensorAvailable[ID_STEPCOUNTER] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwStepcounterSensor: %s", e.what());
        data->sensorAvailable[ID_STEPCOUNTER] = FALSE;
    }
    try {
        data->temperature_sensor = std::make_shared<waydroid::core::SensorfwTemperatureSensor>(dbus_address);
        data->sensorAvailable[ID_TEMPERATURE] = TRUE;
    } catch (std::exception const &e) {
        GINFO("Failed to create SensorfwTemperatureSensor: %s", e.what());
        data->sensorAvailable[ID_TEMPERATURE] = FALSE;
    }
}

void SensorFW::RegisterSensors(sensor_event_cb_t cb, void *userdata) {
    if (data->sensorAvailable[ID_ACCELEROMETER]) {
        mRegistrations.push_back(
            data->accelerometer_sensor->register_accelerometer_handler(
                [this, cb, userdata](AccelerationData value) {
                    this->data->accelerometer_event = value;
                    cb(userdata, ID_ACCELEROMETER);
                }));
    }
    if (data->sensorAvailable[ID_GYROSCOPE]) {
        mRegistrations.push_back(
            data->gyroscope_sensor->register_gyroscope_handler(
                [this, cb, userdata](TimedXyzData value) {
                    this->data->gyroscope_event = value;
                    cb(userdata, ID_GYROSCOPE);
                }));
    }
    if (data->sensorAvailable[ID_HUMIDITY]) {
        mRegistrations.push_back(
            data->humidity_sensor->register_humidity_handler(
                [this, cb, userdata](TimedUnsigned value) {
                    this->data->humidity_event = value;
                    cb(userdata, ID_HUMIDITY);
                }));
    }
    if (data->sensorAvailable[ID_LIGHT]) {
        mRegistrations.push_back(
            data->light_sensor->register_light_handler(
                [this, cb, userdata](TimedUnsigned value) {
                    this->data->light_event = value;
                    cb(userdata, ID_LIGHT);
                }));
    }
    if (data->sensorAvailable[ID_MAGNETIC_FIELD]) {
        mRegistrations.push_back(
            data->magnetometer_sensor->register_magnetometer_handler(
                [this, cb, userdata](CalibratedMagneticFieldData value) {
                    this->data->magnetometer_event = value;
                    cb(userdata, ID_MAGNETIC_FIELD);
                }));
    }
    if (data->sensorAvailable[ID_DEVICE_ORIENTATION]) {
        mRegistrations.push_back(
            data->orientation_sensor->register_orientation_handler(
                [this, cb, userdata](PoseData value) {
                    this->data->orientation_event = value;
                    cb(userdata, ID_DEVICE_ORIENTATION);
                }));
    }
    if (data->sensorAvailable[ID_PRESSURE]) {
        mRegistrations.push_back(
            data->pressure_sensor->register_pressure_handler(
                [this, cb, userdata](TimedUnsigned value) {
                    this->data->pressure_event = value;
                    cb(userdata, ID_PRESSURE);
                }));
    }
    if (data->sensorAvailable[ID_PROXIMITY]) {
        mRegistrations.push_back(
            data->proximity_sensor->register_proximity_handler(
                [this, cb, userdata](ProximityData value) {
                    this->data->proximity_event = value;
                    cb(userdata, ID_PROXIMITY);
                }));
    }
    if (data->sensorAvailable[ID_STEPCOUNTER]) {
        mRegistrations.push_back(
            data->stepcounter_sensor->register_stepcounter_handler(
                [this, cb, userdata](TimedUnsigned value) {
                    this->data->stepcounter_event = value;
                    cb(userdata, ID_STEPCOUNTER);
                }));
    }
    if (data->sensorAvailable[ID_TEMPERATURE]) {
        mRegistrations.push_back(
            data->temperature_sensor->register_temperature_handler(
                [this, cb, userdata](TimedUnsigned value) {
                    this->data->temperature_event = value;
                    cb(userdata, ID_TEMPERATURE);
                }));
    }
}

bool SensorFW::IsSensorAvailable(int id) {
    if (id >= MAX_NUM_SENSORS)
        return false;

    return data->sensorAvailable[id];
}

bool SensorFW::IsSensorEventEnable(int id) {
    if (!IsSensorAvailable(id))
        return false;

    return data->sensorEventEnable[id];
}

int SensorFW::EnableSensorEvents(int id) {
    if (!IsSensorAvailable(id))
        return -ENODEV;

    switch (id) {
    case ID_ACCELEROMETER:
        data->accelerometer_sensor->enable_accelerometer_events();
        break;
    case ID_GYROSCOPE:
        data->gyroscope_sensor->enable_gyroscope_events();
        break;
    case ID_HUMIDITY:
        data->humidity_sensor->enable_humidity_events();
        break;
    case ID_LIGHT:
        data->light_sensor->enable_light_events();
        break;
    case ID_MAGNETIC_FIELD:
        data->magnetometer_sensor->enable_magnetometer_events();
        break;
    case ID_MAGNETIC_FIELD_UNCALIBRATED:
        data->magnetometer_sensor->enable_magnetometer_events();
        break;
    case ID_DEVICE_ORIENTATION:
        data->orientation_sensor->enable_orientation_events();
        break;
    case ID_PRESSURE:
        data->pressure_sensor->enable_pressure_events();
        break;
    case ID_PROXIMITY:
        data->proximity_sensor->enable_proximity_events();
        break;
    case ID_STEPCOUNTER:
        data->stepcounter_sensor->enable_stepcounter_events();
        break;
    case ID_TEMPERATURE:
        data->temperature_sensor->enable_temperature_events();
        break;
    default:
        return -EINVAL;
        break;
    }
    data->sensorEventEnable[id] = TRUE;

    return 0;
}

int SensorFW::DisableSensorEvents(int id) {
    if (!IsSensorAvailable(id))
        return -ENODEV;

    switch (id) {
    case ID_ACCELEROMETER:
        data->accelerometer_sensor->disable_accelerometer_events();
        break;
    case ID_GYROSCOPE:
        data->gyroscope_sensor->disable_gyroscope_events();
        break;
    case ID_HUMIDITY:
        data->humidity_sensor->disable_humidity_events();
        break;
    case ID_LIGHT:
        data->light_sensor->disable_light_events();
        break;
    case ID_MAGNETIC_FIELD:
        data->magnetometer_sensor->disable_magnetometer_events();
        break;
    case ID_MAGNETIC_FIELD_UNCALIBRATED:
        data->magnetometer_sensor->disable_magnetometer_events();
        break;
    case ID_DEVICE_ORIENTATION:
        data->orientation_sensor->disable_orientation_events();
        break;
    case ID_PRESSURE:
        data->pressure_sensor->disable_pressure_events();
        break;
    case ID_PROXIMITY:
        data->proximity_sensor->disable_proximity_events();
        break;
    case ID_STEPCOUNTER:
        data->stepcounter_sensor->disable_stepcounter_events();
        break;
    case ID_TEMPERATURE:
        data->temperature_sensor->disable_temperature_events();
        break;
    default:
        return -EINVAL;
        break;
    }
    data->sensorEventEnable[id] = FALSE;

    return 0;
}

int SensorFW::GetAccelerometerEvent(uint64_t *ts, int *x, int *y, int *z) {
    if (!IsSensorEventEnable(ID_ACCELEROMETER))
        return -EPERM;

    *ts = data->accelerometer_event.timestamp_;
    *x = data->accelerometer_event.x_;
    *y = data->accelerometer_event.y_;
    *z = data->accelerometer_event.z_;

    return 0;
}

int SensorFW::GetGyroscopeEvent(uint64_t *ts, int *x, int *y, int *z) {
    if (!IsSensorEventEnable(ID_GYROSCOPE))
        return -EPERM;

    *ts = data->gyroscope_event.timestamp_;
    *x = data->gyroscope_event.x_;
    *y = data->gyroscope_event.y_;
    *z = data->gyroscope_event.z_;

    return 0;
}

int SensorFW::GetHumidityEvent(uint64_t *ts, unsigned *value) {
    if (!IsSensorEventEnable(ID_HUMIDITY))
        return -EPERM;

    *ts = data->humidity_event.timestamp_;
    *value = data->humidity_event.value_;

    return 0;
}

int SensorFW::GetLightEvent(uint64_t *ts, unsigned *value) {
    if (!IsSensorEventEnable(ID_LIGHT))
        return -EPERM;

    *ts = data->light_event.timestamp_;
    *value = data->light_event.value_;

    return 0;
}

int SensorFW::GetMagnetometerEvent(uint64_t *ts, int *x, int *y, int *z,
        int *rx, int *ry, int *rz, int *level) {
    if (!IsSensorEventEnable(ID_MAGNETIC_FIELD) &&
        !IsSensorEventEnable(ID_MAGNETIC_FIELD_UNCALIBRATED))
        return -EPERM;

    *ts = data->magnetometer_event.timestamp_;
    *x = data->magnetometer_event.x_;
    *y = data->magnetometer_event.y_;
    *z = data->magnetometer_event.z_;
    *rx = data->magnetometer_event.rx_;
    *ry = data->magnetometer_event.ry_;
    *rz = data->magnetometer_event.rz_;
    *level = data->magnetometer_event.level_;

    return 0;
}

int SensorFW::GetOrientationEvent(uint64_t *ts, int *degree) {
    if (!IsSensorEventEnable(ID_DEVICE_ORIENTATION))
        return -EPERM;

    *ts = data->orientation_event.timestamp_;
    switch (data->orientation_event.orientation_)
    {
    case PoseData::Orientation::RightUp:
        *degree = 1;
        break;
    case PoseData::Orientation::BottomUp:
        *degree = 2;
        break;
    case PoseData::Orientation::LeftUp:
        *degree = 3;
        break;
    default:
        *degree = 0;
        break;
    }

    return 0;
}

int SensorFW::GetPressureEvent(uint64_t *ts, unsigned *value) {
    if (!IsSensorEventEnable(ID_PRESSURE))
        return -EPERM;

    *ts = data->pressure_event.timestamp_;
    *value = data->pressure_event.value_;

    return 0;
}

int SensorFW::GetProximityEvent(uint64_t *ts, unsigned *value, bool *isNear) {
    if (!IsSensorEventEnable(ID_PROXIMITY))
        return -EPERM;

    *ts = data->proximity_event.timestamp_;
    *value = data->proximity_event.value_;
    *isNear = data->proximity_event.withinProximity_;

    return 0;
}

int SensorFW::GetStepcounterEvent(uint64_t *ts, unsigned *value) {
    if (!IsSensorEventEnable(ID_STEPCOUNTER))
        return -EPERM;

    *ts = data->stepcounter_event.timestamp_;
    *value = data->stepcounter_event.value_;

    return 0;
}

int SensorFW::GetTemperatureEvent(uint64_t *ts, unsigned *value) {
    if (!IsSensorEventEnable(ID_TEMPERATURE))
        return -EPERM;

    *ts = data->temperature_event.timestamp_;
    *value = data->temperature_event.value_;

    return 0;
}

} // namespace waydroid
