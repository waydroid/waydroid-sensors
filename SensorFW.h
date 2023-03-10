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

#ifndef SENSORHW_H_
#define SENSORHW_H_

#include <plugins/sensorfw_accelerometer_sensor.h>
#include <plugins/sensorfw_gyroscope_sensor.h>
#include <plugins/sensorfw_humidity_sensor.h>
#include <plugins/sensorfw_light_sensor.h>
#include <plugins/sensorfw_magnetometer_sensor.h>
#include <plugins/sensorfw_orientation_sensor.h>
#include <plugins/sensorfw_pressure_sensor.h>
#include <plugins/sensorfw_proximity_sensor.h>
#include <plugins/sensorfw_stepcounter_sensor.h>
#include <plugins/sensorfw_temperature_sensor.h>

#include <vector>

namespace waydroid {

#define MAX_NUM_SENSORS 11

#define SUPPORTED_SENSORS  ((1<<MAX_NUM_SENSORS)-1)

#define  ID_BASE                        0
#define  ID_ACCELEROMETER               (ID_BASE+0)
#define  ID_GYROSCOPE                   (ID_BASE+1)
#define  ID_HUMIDITY                    (ID_BASE+2)
#define  ID_LIGHT                       (ID_BASE+3)
#define  ID_MAGNETIC_FIELD              (ID_BASE+4)
#define  ID_MAGNETIC_FIELD_UNCALIBRATED (ID_BASE+5)
#define  ID_DEVICE_ORIENTATION          (ID_BASE+6)
#define  ID_PRESSURE                    (ID_BASE+7)
#define  ID_PROXIMITY                   (ID_BASE+8)
#define  ID_STEPCOUNTER                 (ID_BASE+9)
#define  ID_TEMPERATURE                 (ID_BASE+10)

#define  SENSORS_ACCELEROMETER                (1 << ID_ACCELEROMETER)
#define  SENSORS_GYROSCOPE                    (1 << ID_GYROSCOPE)
#define  SENSORS_HUMIDITY                     (1 << ID_HUMIDITY)
#define  SENSORS_LIGHT                        (1 << ID_LIGHT)
#define  SENSORS_MAGNETIC_FIELD               (1 << ID_MAGNETIC_FIELD)
#define  SENSORS_MAGNETIC_FIELD_UNCALIBRATED  (1 << ID_MAGNETIC_FIELD_UNCALIBRATED)
#define  SENSORS_DEVICE_ORIENTATION           (1 << ID_DEVICE_ORIENTATION)
#define  SENSORS_PRESSURE                     (1 << ID_PRESSURE)
#define  SENSORS_PROXIMITY                    (1 << ID_PROXIMITY)
#define  SENSORS_STEPCOUNTER                  (1 << ID_STEPCOUNTER)
#define  SENSORS_TEMPERATURE                  (1 << ID_TEMPERATURE)

#define  ID_CHECK(x)  ((unsigned)((x) - ID_BASE) < MAX_NUM_SENSORS)

#define  SENSORS_LIST  \
    SENSOR_(ACCELEROMETER,"accelerometer") \
    SENSOR_(GYROSCOPE,"gyroscope") \
    SENSOR_(HUMIDITY, "humidity") \
    SENSOR_(LIGHT, "light") \
    SENSOR_(MAGNETIC_FIELD,"magnetic-field") \
    SENSOR_(MAGNETIC_FIELD_UNCALIBRATED,"magnetic-field-uncalibrated") \
    SENSOR_(DEVICE_ORIENTATION,"device-orientation") \
    SENSOR_(PRESSURE, "pressure") \
    SENSOR_(PROXIMITY,"proximity") \
    SENSOR_(STEPCOUNTER, "stepcounter") \
    SENSOR_(TEMPERATURE,"temperature")

static const struct {
    const char*  name;
    int          id; } _sensorIds[MAX_NUM_SENSORS] =
{
#define SENSOR_(x,y)  { y, ID_##x },
    SENSORS_LIST
#undef  SENSOR_
};

static const char* _SensorIdToName(int id) {
    int nn;
    for (nn = 0; nn < MAX_NUM_SENSORS; nn++)
        if (id == _sensorIds[nn].id)
            return _sensorIds[nn].name;
    return "<UNKNOWN>";
}

typedef struct {
    /* General */
    gboolean sensorAvailable[MAX_NUM_SENSORS];
    gboolean sensorEventEnable[MAX_NUM_SENSORS];

    /* Sensors */
    std::shared_ptr<waydroid::core::SensorfwAccelerometerSensor> accelerometer_sensor;
    std::shared_ptr<waydroid::core::SensorfwGyroscopeSensor> gyroscope_sensor;
    std::shared_ptr<waydroid::core::SensorfwHumiditySensor> humidity_sensor;
    std::shared_ptr<waydroid::core::SensorfwLightSensor> light_sensor;
    std::shared_ptr<waydroid::core::SensorfwMagnetometerSensor> magnetometer_sensor;
    std::shared_ptr<waydroid::core::SensorfwOrientationSensor> orientation_sensor;
    std::shared_ptr<waydroid::core::SensorfwPressureSensor> pressure_sensor;
    std::shared_ptr<waydroid::core::SensorfwProximitySensor> proximity_sensor;
    std::shared_ptr<waydroid::core::SensorfwStepcounterSensor> stepcounter_sensor;
    std::shared_ptr<waydroid::core::SensorfwTemperatureSensor> temperature_sensor;

    /* Events */
    AccelerationData accelerometer_event;
    TimedXyzData gyroscope_event;
    TimedUnsigned humidity_event;
    TimedUnsigned light_event;
    CalibratedMagneticFieldData magnetometer_event;
    PoseData orientation_event;
    TimedUnsigned pressure_event;
    ProximityData proximity_event;
    TimedUnsigned stepcounter_event;
    TimedUnsigned temperature_event;
} SensorData;

typedef void (*sensor_event_cb_t)(void *userdata, int id);

struct SensorFW {
    SensorFW();

    void RegisterSensors(sensor_event_cb_t cb, void *userdata);
    bool IsSensorAvailable(int id);
    bool IsSensorEventEnable(int id);
    int EnableSensorEvents(int id);
    int DisableSensorEvents(int id);

    int GetAccelerometerEvent(uint64_t *ts, int *x, int *y, int *z);
    int GetGyroscopeEvent(uint64_t *ts, int *x, int *y, int *z);
    int GetHumidityEvent(uint64_t *ts, unsigned *value);
    int GetLightEvent(uint64_t *ts, unsigned *value);
    int GetMagnetometerEvent(uint64_t *ts, int *x, int *y, int *z,
        int *rx, int *ry, int *rz, int* level);
    int GetOrientationEvent(uint64_t *ts, int *degree);
    int GetPressureEvent(uint64_t *ts, unsigned *value);
    int GetProximityEvent(uint64_t *ts, unsigned *value, bool *isNear);
    int GetStepcounterEvent(uint64_t *ts, unsigned *value);
    int GetTemperatureEvent(uint64_t *ts, unsigned *value);

private:
    SensorData *data;
    std::vector<waydroid::core::HandlerRegistration> mRegistrations;
};

}  // namespace waydroid

#endif  // SENSORHW_H_
