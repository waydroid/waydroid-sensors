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

#include "Sensors.h"

#include <pthread.h>

namespace waydroid {
namespace sensors {
namespace implementation {

/* return the current time in nanoseconds */
static int64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return (int64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

/* Pick up one pending sensor event. On success, this returns the sensor
 * id, and sets |*event| accordingly. On failure, i.e. if there are no
 * pending events, return -EINVAL.
 *
 * Note: The device's lock must be acquired.
 */
static int sensor_device_pick_pending_event_locked(SensorDevice *d,
                                                   sensors_event_t* event)
{
    uint32_t mask = SUPPORTED_SENSORS & d->pendingSensors;
    if (mask) {
        uint32_t i = 31 - __builtin_clz(mask);
        d->pendingSensors &= ~(1U << i);
        // Copy the structure
        *event = d->sensors[i];

        if (d->sensors[i].sensorType == SENSOR_TYPE_META_DATA) {
            if (d->flush_count[i] > 0) {
                // Another 'flush' is queued after this one.
                // Don't clear this event; just decrement the count.
                (d->flush_count[i])--;
                // And re-mark it as pending
                d->pendingSensors |= (1U << i);
            } else {
                // We are done flushing
                // sensor_event_cb() will leave
                // the meta-data in place until we have it.
                // Set |type| to something other than META_DATA
                // so sensor_event_cb() can
                // continue.
                d->sensors[i].sensorType = SENSOR_TYPE_ACCELEROMETER;
            }
        } else {
            event->sensorHandle = i;
            //event->version = sizeof(*event);
        }

        return i;
    }
    GERR("No sensor to return!!! pendingSensors=0x%08x", d->pendingSensors);
    // we may end-up in a busy loop, slow things down, just in case.
    usleep(1000);
    return -EINVAL;
}

/* Block until new sensor events are reported by the emulator, or if a
 * 'wake' command is received through the service. On succes, return 0
 * and updates the |pendingEvents| and |sensors| fields of |dev|.
 * On failure, return -errno.
 *
 * Note: The device lock must be acquired when calling this function, and
 *       will still be held on return. However, the function releases the
 *       lock temporarily during the blocking wait.
 */
static void sensor_event_cb(void *userdata, int id)
{
    SensorDevice* dev = (SensorDevice*) userdata;
    // Accumulate pending events into |events| and |new_sensors| mask
    // until a 'sync' or 'wake' command is received. This also simplifies the
    // code a bit.
    uint32_t new_sensors = 0U;
    sensors_event_t* events = dev->sensors;

    int64_t event_time = -1;
    uint64_t ts;
    int x, y, z, rx, ry, rz, tmp;
    unsigned value;
    bool isNear;

    // If the existing entry for this sensor is META_DATA,
    // do not overwrite it. We can resume saving sensor
    // values after that meta data has been received.

    switch (id) {
        case ID_ACCELEROMETER:
            if (dev->mSensorFWDevice->GetAccelerometerEvent(&ts, &x, &y, &z) == 0) {
                if (ts != dev->last_TimeStamp[ID_ACCELEROMETER]) {
                    new_sensors |= SENSORS_ACCELEROMETER;
                    events[ID_ACCELEROMETER].u.vec3.x = x / 100.00f;
                    events[ID_ACCELEROMETER].u.vec3.y = y / 100.00f;
                    events[ID_ACCELEROMETER].u.vec3.z = z / 100.00f;
                    events[ID_ACCELEROMETER].u.vec3.status = ACCURACY_MEDIUM;
                    events[ID_ACCELEROMETER].sensorType = SENSOR_TYPE_ACCELEROMETER;
                    dev->last_TimeStamp[ID_ACCELEROMETER] = ts;
                }
            }
            break;
        case ID_GYROSCOPE:
            if (dev->mSensorFWDevice->GetGyroscopeEvent(&ts, &x, &y, &z) == 0) {
                if (ts != dev->last_TimeStamp[ID_GYROSCOPE]) {
                    new_sensors |= SENSORS_GYROSCOPE;
                    events[ID_GYROSCOPE].u.vec3.x = x / 1000.000f;
                    events[ID_GYROSCOPE].u.vec3.y = y / 1000.000f;
                    events[ID_GYROSCOPE].u.vec3.z = z / 1000.000f;
                    events[ID_GYROSCOPE].u.vec3.status = ACCURACY_MEDIUM;
                    events[ID_GYROSCOPE].sensorType = SENSOR_TYPE_GYROSCOPE;
                    dev->last_TimeStamp[ID_GYROSCOPE] = ts;
                }
            }
            break;
        case ID_HUMIDITY:
            if (dev->mSensorFWDevice->GetHumidityEvent(&ts, &value) == 0) {
                if (ts != dev->last_TimeStamp[ID_HUMIDITY]) {
                    new_sensors |= SENSORS_HUMIDITY;
                    events[ID_HUMIDITY].u.scalar = value;
                    events[ID_HUMIDITY].sensorType = SENSOR_TYPE_RELATIVE_HUMIDITY;
                    dev->last_TimeStamp[ID_HUMIDITY] = ts;
                }
            }
            break;
        case ID_LIGHT:
            if (dev->mSensorFWDevice->GetLightEvent(&ts, &value) == 0) {
                if (ts != dev->last_TimeStamp[ID_LIGHT]) {
                    new_sensors |= SENSORS_LIGHT;
                    events[ID_LIGHT].u.scalar = value;
                    events[ID_LIGHT].sensorType = SENSOR_TYPE_LIGHT;
                    dev->last_TimeStamp[ID_LIGHT] = ts;
                }
            }
            break;
        case ID_MAGNETIC_FIELD:
            if (dev->mSensorFWDevice->GetMagnetometerEvent(&ts, &x, &y, &z, &rx, &ry, &rz, &tmp) == 0) {
                if (ts != dev->last_TimeStamp[ID_MAGNETIC_FIELD]) {
                    new_sensors |= SENSORS_MAGNETIC_FIELD;
                    events[ID_MAGNETIC_FIELD].u.vec3.x = x;
                    events[ID_MAGNETIC_FIELD].u.vec3.y = y;
                    events[ID_MAGNETIC_FIELD].u.vec3.z = z;
                    events[ID_MAGNETIC_FIELD].u.vec3.status = ACCURACY_HIGH;
                    events[ID_MAGNETIC_FIELD].sensorType = SENSOR_TYPE_MAGNETIC_FIELD;
                    dev->last_TimeStamp[ID_MAGNETIC_FIELD] = ts;
                }
                if (ts != dev->last_TimeStamp[ID_MAGNETIC_FIELD_UNCALIBRATED]) {
                    new_sensors |= SENSORS_MAGNETIC_FIELD_UNCALIBRATED;
                    events[ID_MAGNETIC_FIELD_UNCALIBRATED].u.vec3.x = rx;
                    events[ID_MAGNETIC_FIELD_UNCALIBRATED].u.vec3.y = ry;
                    events[ID_MAGNETIC_FIELD_UNCALIBRATED].u.vec3.z = rz;
                    events[ID_MAGNETIC_FIELD_UNCALIBRATED].u.vec3.status = ACCURACY_HIGH;
                    events[ID_MAGNETIC_FIELD_UNCALIBRATED].sensorType = SENSOR_TYPE_MAGNETIC_FIELD;
                    dev->last_TimeStamp[ID_MAGNETIC_FIELD_UNCALIBRATED] = ts;
                }
            }
            break;
        case ID_DEVICE_ORIENTATION:
            if (dev->mSensorFWDevice->GetOrientationEvent(&ts, &tmp) == 0) {
                if (ts != dev->last_TimeStamp[ID_DEVICE_ORIENTATION]) {
                    new_sensors |= SENSORS_DEVICE_ORIENTATION;
                    events[ID_DEVICE_ORIENTATION].u.scalar = tmp;
                    events[ID_DEVICE_ORIENTATION].sensorType = SENSOR_TYPE_DEVICE_ORIENTATION;
                    dev->last_TimeStamp[ID_DEVICE_ORIENTATION] = ts;
                }
            }
            break;
        case ID_PRESSURE:
            if (dev->mSensorFWDevice->GetPressureEvent(&ts, &value) == 0) {
                if (ts != dev->last_TimeStamp[ID_PRESSURE]) {
                    new_sensors |= SENSORS_PRESSURE;
                    events[ID_PRESSURE].u.scalar = value;
                    events[ID_PRESSURE].sensorType = SENSOR_TYPE_PRESSURE;
                    dev->last_TimeStamp[ID_PRESSURE] = ts;
                }
            }
            break;
        case ID_PROXIMITY:
            if (dev->mSensorFWDevice->GetProximityEvent(&ts, &value, &isNear) == 0) {
                if (ts != dev->last_TimeStamp[ID_PROXIMITY]) {
                    new_sensors |= SENSORS_PROXIMITY;
                    events[ID_PROXIMITY].u.scalar = isNear ? 0 : 5;
                    events[ID_PROXIMITY].sensorType = SENSOR_TYPE_PROXIMITY;
                    dev->last_TimeStamp[ID_PROXIMITY] = ts;
                }
            }
            break;
        case ID_STEPCOUNTER:
            if (dev->mSensorFWDevice->GetStepcounterEvent(&ts, &value) == 0) {
                if (ts != dev->last_TimeStamp[ID_STEPCOUNTER]) {
                    new_sensors |= SENSORS_STEPCOUNTER;
                    events[ID_STEPCOUNTER].u.stepCount = value;
                    events[ID_STEPCOUNTER].sensorType = SENSOR_TYPE_STEP_COUNTER;
                    dev->last_TimeStamp[ID_STEPCOUNTER] = ts;
                }
            }
            break;
        case ID_TEMPERATURE:
            if (dev->mSensorFWDevice->GetTemperatureEvent(&ts, &value) == 0) {
                if (ts != dev->last_TimeStamp[ID_TEMPERATURE]) {
                    new_sensors |= SENSORS_TEMPERATURE;
                    events[ID_TEMPERATURE].u.scalar = value;
                    events[ID_TEMPERATURE].sensorType = SENSOR_TYPE_AMBIENT_TEMPERATURE;
                    dev->last_TimeStamp[ID_TEMPERATURE] = ts;
                }
            }
            break;
        default:
            break;
    }

    if (new_sensors) {
        /* update the time of each new sensor event. */
        dev->pendingSensors |= new_sensors;
        int64_t t = (event_time < 0) ? 0 : event_time * 1000LL;

        /* Use the time at the first "sync:" as the base for later
         * time values.
         * CTS tests require sensors to return an event timestamp (sync) that is
         * strictly before the time of the event arrival. We don't actually have
         * a time syncronization protocol here, and the only data point is the
         * "sync:" timestamp - which is an emulator's timestamp of a clock that
         * is synced with the guest clock, and it only the timestamp after all
         * events were sent.
         * To make it work, let's compare the calculated timestamp with current
         * time and take the lower value - we don't believe in events from the
         * future anyway.
         */
        const int64_t now = now_ns();

        if (dev->timeStart == 0) {
            dev->timeStart  = now;
            dev->timeOffset = dev->timeStart - t;
        }
        t += dev->timeOffset;
        if (t > now) {
            t = now;
        }

        while (new_sensors) {
            uint32_t i = 31 - __builtin_clz(new_sensors);
            new_sensors &= ~(1U << i);
            dev->sensors[i].timestamp = t;
        }
    }
    if (dev->waiting_for_data)
        g_main_loop_quit(dev->loop);
}

Sensors::Sensors()
    : mSensorDevice(nullptr) {
    mSensorDevice = (SensorDevice *) malloc(sizeof(*mSensorDevice));
    memset(mSensorDevice, 0, sizeof(*mSensorDevice));

    // (sensorType == SENSOR_TYPE_META_DATA) is
    // sticky. Don't start off with that setting.
    for (int idx = 0; idx < MAX_NUM_SENSORS; idx++) {
        mSensorDevice->sensors[idx].sensorType = SENSOR_TYPE_ACCELEROMETER;
        mSensorDevice->flush_count[idx] = 0;
    }

    mSensorDevice->mSensorFWDevice = new SensorFW();
    mSensorDevice->mSensorFWDevice->RegisterSensors(sensor_event_cb, mSensorDevice);

    pthread_mutex_init(&mSensorDevice->lock, NULL);
    mSensorDevice->loop = g_main_loop_new(NULL, TRUE);
}

std::vector<sensor_t> Sensors::getSensorsList() {
    std::vector<sensor_t> out_vector;

    int sensors_count = 0;
    
    for (int id = 0; id < MAX_NUM_SENSORS; id++) {
        if(!mSensorDevice->mSensorFWDevice->IsSensorAvailable(id)) {
            GERR("Sensor %s Not found!", waydroid::_SensorIdToName(id));
            continue;
        }
        sensor_t sensor_info;
        sensor_info.handle = id;
        switch (id)
        {
        case ID_ACCELEROMETER:
            sensor_info.name.data.str = "SensorFW 3-axis Accelerometer";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_ACCELEROMETER;
            sensor_info.typeAsString.data.str = "android.sensor.accelerometer";
            sensor_info.maxRange = 39.3;
            sensor_info.resolution = 1.0 / 4032.0;
            sensor_info.power = 3.0;
            sensor_info.minDelay = 10000;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 500000;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_CONTINUOUS_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_GYROSCOPE:
            sensor_info.name.data.str = "SensorFW 3-axis Gyroscope";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_GYROSCOPE;
            sensor_info.typeAsString.data.str = "android.sensor.gyroscope";
            sensor_info.maxRange = 16.46;
            sensor_info.resolution = 1.0 / 1000.0;
            sensor_info.power = 3.0;
            sensor_info.minDelay = 10000;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 500000;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_CONTINUOUS_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_HUMIDITY:
            sensor_info.name.data.str = "SensorFW Humidity sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_RELATIVE_HUMIDITY;
            sensor_info.typeAsString.data.str = "android.sensor.relative_humidity";
            sensor_info.maxRange = 100.0;
            sensor_info.resolution = 1.0;
            sensor_info.power = 20.0;
            sensor_info.minDelay = 0;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 0;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_ON_CHANGE_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_LIGHT:
            sensor_info.name.data.str = "SensorFW Light sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_LIGHT;
            sensor_info.typeAsString.data.str = "android.sensor.light";
            sensor_info.maxRange = 40000.0;
            sensor_info.resolution = 1.0;
            sensor_info.power = 20.0;
            sensor_info.minDelay = 0;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 0;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_ON_CHANGE_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_MAGNETIC_FIELD:
            sensor_info.name.data.str = "SensorFW 3-axis Magnetic field sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_MAGNETIC_FIELD;
            sensor_info.typeAsString.data.str = "android.sensor.magnetic_field";
            sensor_info.maxRange = 2000.0;
            sensor_info.resolution = .5;
            sensor_info.power = 6.7;
            sensor_info.minDelay = 10000;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 500000;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_CONTINUOUS_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_MAGNETIC_FIELD_UNCALIBRATED:
            sensor_info.name.data.str = "SensorFW 3-axis Magnetic field sensor (uncalibrated)";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
            sensor_info.typeAsString.data.str = "android.sensor.magnetic_field_uncalibrated";
            sensor_info.maxRange = 2000.0;
            sensor_info.resolution = 0.5;
            sensor_info.power = 6.7;
            sensor_info.minDelay = 10000;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 500000;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_CONTINUOUS_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_DEVICE_ORIENTATION:
            sensor_info.name.data.str = "SensorFW Device Orientation sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_DEVICE_ORIENTATION;
            sensor_info.typeAsString.data.str = "android.sensor.device_orientation";
            sensor_info.maxRange = 3.0;
            sensor_info.resolution = 1.0;
            sensor_info.power = 0.1;
            sensor_info.minDelay = 0;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 0;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_ON_CHANGE_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_PRESSURE:
            sensor_info.name.data.str = "SensorFW Pressure sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_PRESSURE;
            sensor_info.typeAsString.data.str = "android.sensor.pressure";
            sensor_info.maxRange = 800.0;
            sensor_info.resolution = 1.0;
            sensor_info.power = 20.0;
            sensor_info.minDelay = 10000;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 500000;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_CONTINUOUS_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_PROXIMITY:
            sensor_info.name.data.str = "SensorFW Proximity sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_PROXIMITY;
            sensor_info.typeAsString.data.str = "android.sensor.proximity";
            sensor_info.maxRange = 5.0;
            sensor_info.resolution = 5.0;
            sensor_info.power = 20.0;
            sensor_info.minDelay = 0;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 0;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_ON_CHANGE_MODE |
                                SENSOR_FLAG_WAKE_UP;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_STEPCOUNTER:
            sensor_info.name.data.str = "SensorFW Step counter sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_STEP_COUNTER;
            sensor_info.typeAsString.data.str = "android.sensor.step_counter";
            sensor_info.maxRange = 1.0;
            sensor_info.resolution = 1.0;
            sensor_info.power = 0.0;
            sensor_info.minDelay = 0;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 0;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_ON_CHANGE_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;
            out_vector.push_back(sensor_info);
            break;
        case ID_TEMPERATURE:
            sensor_info.name.data.str = "SensorFW Ambient Temperature sensor";
            sensor_info.vendor.data.str = kWaydroidVendor;
            sensor_info.version = 1;
            sensor_info.type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
            sensor_info.typeAsString.data.str = "android.sensor.ambient_temperature";
            sensor_info.maxRange = 80.0;
            sensor_info.resolution = 1.0;
            sensor_info.power = 0.0;
            sensor_info.minDelay = 0;
            sensor_info.fifoReservedEventCount = 0;
            sensor_info.fifoMaxEventCount = 0;
            sensor_info.requiredPermission.data.str = "";
            sensor_info.maxDelay = 0;
            sensor_info.flags = SENSOR_FLAG_DATA_INJECTION |
                                SENSOR_FLAG_ON_CHANGE_MODE;
            sensor_info.name.len = strlen(sensor_info.name.data.str);
            sensor_info.name.owns_buffer = TRUE;
            sensor_info.vendor.len = strlen(sensor_info.vendor.data.str);
            sensor_info.vendor.owns_buffer = TRUE;
            sensor_info.typeAsString.len = strlen(sensor_info.typeAsString.data.str);
            sensor_info.typeAsString.owns_buffer = TRUE;
            sensor_info.requiredPermission.len = strlen(sensor_info.requiredPermission.data.str);
            sensor_info.requiredPermission.owns_buffer = TRUE;    
            out_vector.push_back(sensor_info);
            break;
        default:
            break;
        }
    }

    return out_vector;
}

int Sensors::activate(
        int32_t handle, bool enabled) {

    /* Sanity check */
    if (!ID_CHECK(handle)) {
        GERR("activate: bad handle ID: %d", handle);
        return RESULT_BAD_VALUE;
    }

    /* Exit early if sensor is already enabled/disabled. */
    uint32_t mask = (1U << handle);
    uint32_t sensors = enabled ? mask : 0;

    pthread_mutex_lock(&mSensorDevice->lock);

    uint32_t active = mSensorDevice->active_sensors;
    uint32_t new_sensors = (active & ~mask) | (sensors & mask);
    uint32_t changed = active ^ new_sensors;

    if (changed) {
        if (enabled)
            mSensorDevice->mSensorFWDevice->EnableSensorEvents(handle);
        else
            mSensorDevice->mSensorFWDevice->DisableSensorEvents(handle);
        mSensorDevice->active_sensors = new_sensors;
    }
    pthread_mutex_unlock(&mSensorDevice->lock);
    return RESULT_OK;
}

std::vector<sensors_event_t> Sensors::poll(int32_t maxCount, int *err_out) {
    std::vector<sensors_event_t> out;
    int err = 0;

    if (maxCount <= 0) {
        err = -EINVAL;
    } else {
        int bufferSize = maxCount <= kPollMaxBufferSize ? maxCount : kPollMaxBufferSize;

        if (!mSensorDevice->pendingSensors) {
            mSensorDevice->waiting_for_data = true;
            g_main_loop_run(mSensorDevice->loop);
            mSensorDevice->waiting_for_data = false;
        }
        out.resize(bufferSize);

        /* Now read as many pending events as needed. */
        for (int i = 0; i < bufferSize; i++)  {
            if (!mSensorDevice->pendingSensors) {
                break;
            }
            int ret = sensor_device_pick_pending_event_locked(mSensorDevice, &out[i]);
            if (ret < 0) {
                if (!err) {
                    err = ret;
                }
                break;
            }
            err++;
        }
    }

out:
    if (err < 0) {
        *err_out = RESULT_BAD_VALUE;
        return out;
    }

    const size_t count = (size_t)err;
    out.resize(count);

    *err_out = RESULT_OK;
    return out;
}

int Sensors::flush(int32_t handle) {
    /* Sanity check */
    if (!ID_CHECK(handle)) {
        GERR("bad handle ID");
        return RESULT_BAD_VALUE;
    }

    pthread_mutex_lock(&mSensorDevice->lock);
    if ((mSensorDevice->pendingSensors & (1U << handle)) &&
        mSensorDevice->sensors[handle].sensorType == SENSOR_TYPE_META_DATA)
    {
        // A 'flush' operation is already pending. Just increment the count.
        (mSensorDevice->flush_count[handle])++;
    } else {
        mSensorDevice->flush_count[handle] = 0;
        mSensorDevice->sensors[handle].sensorType = SENSOR_TYPE_META_DATA;
        mSensorDevice->sensors[handle].timestamp = 0;
        mSensorDevice->sensors[handle].sensorHandle = handle;
        mSensorDevice->sensors[handle].u.meta.what = META_DATA_FLUSH_COMPLETE;
        mSensorDevice->pendingSensors |= (1U << handle);
    }
    pthread_mutex_unlock(&mSensorDevice->lock);
    
    return RESULT_OK;
}

void Sensors::killLoops() {
    if (mSensorDevice->waiting_for_data)
        g_main_loop_quit(mSensorDevice->loop);
}

}  // namespace implementation
}  // namespace sensors
}  // namespace waydroid
