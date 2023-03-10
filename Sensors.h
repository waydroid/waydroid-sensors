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

#ifndef ANDBOX_HARDWARE_SENSORS_H_
#define ANDBOX_HARDWARE_SENSORS_H_

#include <gbinder.h>
#include <gutil_log.h>
#include <glib-unix.h>

#include "hybrisbindertypes.h"
#include "SensorFW.h"

using waydroid::SensorFW;

namespace waydroid {
namespace sensors {
namespace implementation {

constexpr char kWaydroidVendor[] = "The Waydroid Project";

typedef struct SensorDevice {
    SensorFW *mSensorFWDevice;
    uint64_t last_TimeStamp[MAX_NUM_SENSORS];
    sensors_event_t sensors[MAX_NUM_SENSORS];
    uint32_t pendingSensors;
    int64_t timeStart;
    int64_t timeOffset;
    uint32_t active_sensors;
    int flush_count[MAX_NUM_SENSORS];
    pthread_mutex_t lock;
    GMainLoop* loop;
    bool waiting_for_data;
} SensorDevice;

struct Sensors {
    Sensors();

    std::vector<sensor_t> getSensorsList();
    int activate(int32_t handle, bool enabled);
    std::vector<sensors_event_t> poll(int32_t maxCount, int *err_out);
    int flush(int32_t handle);
    void killLoops();

private:
    static constexpr int32_t kPollMaxBufferSize = 128;
    SensorDevice *mSensorDevice;
};

}  // namespace implementation
}  // namespace sensors
}  // namespace waydroid

#endif  // ANDBOX_HARDWARE_SENSORS_H_
