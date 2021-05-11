/****************************************************************************
**
** Copyright (C) 2019 Jolla Ltd
**
**
** $QT_BEGIN_LICENSE:LGPL$
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef HYBRIS_BINDER_TYPES_H
#define HYBRIS_BINDER_TYPES_H

#include <gbinder.h>
#include <stdint.h>
#include <sys/types.h>

#define STATUS_OK 0

#define ALIGNED(x) __attribute__ ((aligned(x)))

enum binder_calls {
    // MUST be in the same order as the interfaces in ISensors.hidl
    GET_SENSORS_LIST = GBINDER_FIRST_CALL_TRANSACTION,
    SET_OPERATION_MODE,
    ACTIVATE,
    POLL,
    BATCH,
    FLUSH,
    INJECT_SENSOR_DATA,
    REGISTER_DIRECT_CHANNEL,
    UNREGISTER_DIRECT_CHANNEL,
    CONFIG_DIRECT_REPORT,
};

enum {
    RESULT_OK = 0,
    RESULT_PERMISSION_DENIED = -1,
    RESULT_NO_MEMORY = -12,
    RESULT_BAD_VALUE = -22,
    RESULT_INVALID_OPERATION = -38,
};

enum {
    OPERATION_MODE_NORMAL = 0,
    OPERATION_MODE_DATA_INJECTION = 1,
};

enum {
    SENSOR_TYPE_META_DATA = 0,
    SENSOR_TYPE_ACCELEROMETER = 1,
    SENSOR_TYPE_MAGNETIC_FIELD = 2,
    SENSOR_TYPE_ORIENTATION = 3,
    SENSOR_TYPE_GYROSCOPE = 4,
    SENSOR_TYPE_LIGHT = 5,
    SENSOR_TYPE_PRESSURE = 6,
    SENSOR_TYPE_TEMPERATURE = 7,
    SENSOR_TYPE_PROXIMITY = 8,
    SENSOR_TYPE_GRAVITY = 9,
    SENSOR_TYPE_LINEAR_ACCELERATION = 10,
    SENSOR_TYPE_ROTATION_VECTOR = 11,
    SENSOR_TYPE_RELATIVE_HUMIDITY = 12,
    SENSOR_TYPE_AMBIENT_TEMPERATURE = 13,
    SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED = 14,
    SENSOR_TYPE_GAME_ROTATION_VECTOR = 15,
    SENSOR_TYPE_GYROSCOPE_UNCALIBRATED = 16,
    SENSOR_TYPE_SIGNIFICANT_MOTION = 17,
    SENSOR_TYPE_STEP_DETECTOR = 18,
    SENSOR_TYPE_STEP_COUNTER = 19,
    SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR = 20,
    SENSOR_TYPE_HEART_RATE = 21,
    SENSOR_TYPE_TILT_DETECTOR = 22,
    SENSOR_TYPE_WAKE_GESTURE = 23,
    SENSOR_TYPE_GLANCE_GESTURE = 24,
    SENSOR_TYPE_PICK_UP_GESTURE = 25,
    SENSOR_TYPE_WRIST_TILT_GESTURE = 26,
    SENSOR_TYPE_DEVICE_ORIENTATION = 27,
    SENSOR_TYPE_POSE_6DOF = 28,
    SENSOR_TYPE_STATIONARY_DETECT = 29,
    SENSOR_TYPE_MOTION_DETECT = 30,
    SENSOR_TYPE_HEART_BEAT = 31,
    SENSOR_TYPE_DYNAMIC_SENSOR_META = 32,
    SENSOR_TYPE_ADDITIONAL_INFO = 33,
    SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT = 34,
    SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED = 35,
    SENSOR_TYPE_DEVICE_PRIVATE_BASE = 65536, // 0x10000
};

enum {
    SENSOR_FLAG_WAKE_UP = 1u, // 1
    SENSOR_FLAG_CONTINUOUS_MODE = 0u, // 0
    SENSOR_FLAG_ON_CHANGE_MODE = 2u, // 2
    SENSOR_FLAG_ONE_SHOT_MODE = 4u, // 4
    SENSOR_FLAG_SPECIAL_REPORTING_MODE = 6u, // 6
    SENSOR_FLAG_DATA_INJECTION = 16u, // 0x10
    SENSOR_FLAG_DYNAMIC_SENSOR = 32u, // 0x20
    SENSOR_FLAG_ADDITIONAL_INFO = 64u, // 0x40
    SENSOR_FLAG_DIRECT_CHANNEL_ASHMEM = 1024u, // 0x400
    SENSOR_FLAG_DIRECT_CHANNEL_GRALLOC = 2048u, // 0x800
    SENSOR_FLAG_MASK_REPORTING_MODE = 14u, // 0xE
    SENSOR_FLAG_MASK_DIRECT_REPORT = 896u, // 0x380
    SENSOR_FLAG_MASK_DIRECT_CHANNEL = 3072u, // 0xC00
};

struct sensor_t {
    int32_t handle ALIGNED(4);
    gbinder_hidl_string name ALIGNED(8);
    gbinder_hidl_string vendor ALIGNED(8);
    int32_t version ALIGNED(4);
    int32_t type ALIGNED(4);
    gbinder_hidl_string typeAsString ALIGNED(8);
    float maxRange ALIGNED(4);
    float resolution ALIGNED(4);
    float power ALIGNED(4);
    int32_t minDelay ALIGNED(4);
    uint32_t fifoReservedEventCount ALIGNED(4);
    uint32_t fifoMaxEventCount ALIGNED(4);
    gbinder_hidl_string requiredPermission ALIGNED(8);
    int32_t maxDelay ALIGNED(4);
    uint32_t flags ALIGNED(4);
} ALIGNED(8);

static_assert(sizeof(sensor_t) == 112, "wrong size");

enum {
    NO_CONTACT = -1, // (-1)
    UNRELIABLE = 0,
    ACCURACY_LOW = 1,
    ACCURACY_MEDIUM = 2,
    ACCURACY_HIGH = 3,
};

struct Vec3 {
    float x ALIGNED(4);
    float y ALIGNED(4);
    float z ALIGNED(4);
    int8_t status ALIGNED(1);
} ALIGNED(4);

static_assert(sizeof(Vec3) == 16, "wrong size");

struct Vec4 {
    float x ALIGNED(4);
    float y ALIGNED(4);
    float z ALIGNED(4);
    float w ALIGNED(4);
} ALIGNED(4);

static_assert(sizeof(Vec4) == 16, "wrong size");

struct Uncal {
    float x ALIGNED(4);
    float y ALIGNED(4);
    float z ALIGNED(4);
    float x_bias ALIGNED(4);
    float y_bias ALIGNED(4);
    float z_bias ALIGNED(4);
} ALIGNED(4);

static_assert(sizeof(Uncal) == 24, "wrong size");

struct HeartRate {
    float bpm ALIGNED(4);
    int8_t status ALIGNED(1);
} ALIGNED(4);

static_assert(sizeof(HeartRate) == 8, "wrong size");

enum {
    META_DATA_FLUSH_COMPLETE = 1u, // 1
};

struct MetaData {
    uint32_t what ALIGNED(4);
} ALIGNED(4);

static_assert(sizeof(MetaData) == 4, "wrong size");

struct Dynamicsensor_t {
    bool connected ALIGNED(1);
    int32_t handle ALIGNED(4);
    uint8_t uuid[16] ALIGNED(1);
} ALIGNED(4);

static_assert(sizeof(Dynamicsensor_t) == 24, "wrong size");

enum class AdditionalInfoType : uint32_t {
    AINFO_BEGIN = 0u, // 0
    AINFO_END = 1u, // 1
    AINFO_UNTRACKED_DELAY = 65536u, // 0x10000
    AINFO_INTERNAL_TEMPERATURE = 65537u, // 65537
    AINFO_VEC3_CALIBRATION = 65538u, // 65538
    AINFO_SENSOR_PLACEMENT = 65539u, // 65539
    AINFO_SAMPLING = 65540u, // 65540
    AINFO_CHANNEL_NOISE = 131072u, // 0x20000
    AINFO_CHANNEL_SAMPLER = 131073u, // 131073
    AINFO_CHANNEL_FILTER = 131074u, // 131074
    AINFO_CHANNEL_LINEAR_TRANSFORM = 131075u, // 131075
    AINFO_CHANNEL_NONLINEAR_MAP = 131076u, // 131076
    AINFO_CHANNEL_RESAMPLER = 131077u, // 131077
    AINFO_LOCAL_GEOMAGNETIC_FIELD = 196608u, // 0x30000
    AINFO_LOCAL_GRAVITY = 196609u, // 196609
    AINFO_DOCK_STATE = 196610u, // 196610
    AINFO_HIGH_PERFORMANCE_MODE = 196611u, // 196611
    AINFO_MAGNETIC_FIELD_CALIBRATION = 196612u, // 196612
    AINFO_CUSTOM_START = 268435456u, // 0x10000000
    AINFO_DEBUGGING_START = 1073741824u, // 0x40000000
};

struct AdditionalInfo {
    union Payload {
        int32_t data_int32[14] ALIGNED(4);
        float data_float[14] ALIGNED(4);
    } ALIGNED(4);

    static_assert(sizeof(AdditionalInfo::Payload) == 56, "wrong size");

    AdditionalInfoType type ALIGNED(4);
    int32_t serial ALIGNED(4);
    AdditionalInfo::Payload u ALIGNED(4);
} ALIGNED(4);

static_assert(sizeof(AdditionalInfo) == 64, "wrong size");

union SensorEventPayload {
    Vec3 vec3 ALIGNED(4);
    Vec4 vec4 ALIGNED(4);
    Uncal uncal ALIGNED(4);
    MetaData meta ALIGNED(4);
    float scalar ALIGNED(4);
    uint64_t stepCount ALIGNED(8);
    HeartRate heartRate ALIGNED(4);
    float pose6DOF[15] ALIGNED(4);
    Dynamicsensor_t dynamic ALIGNED(4);
    AdditionalInfo additional ALIGNED(4);
    float data[16] ALIGNED(4);
} ALIGNED(8);

static_assert(sizeof(SensorEventPayload) == 64, "wrong size");

struct sensors_event_t {
    int64_t timestamp ALIGNED(8);
    int32_t sensorHandle ALIGNED(4);
    int32_t sensorType ALIGNED(4);
    SensorEventPayload u ALIGNED(8);
} ALIGNED(8);

static_assert(sizeof(sensors_event_t) == 80, "wrong size");

enum class RateLevel : int32_t {
    STOP = 0,
    NORMAL = 1,
    FAST = 2,
    VERY_FAST = 3,
};

enum class SensorsEventFormatOffset : uint16_t {
    SIZE_FIELD = 0, // 0x0
    REPORT_TOKEN = 4, // 0x4
    SENSOR_TYPE = 8, // 0x8
    ATOMIC_COUNTER = 12, // 0xC
    TIMESTAMP = 16, // 0x10
    DATA = 24, // 0x18
    RESERVED = 88, // 0x58
    TOTAL_LENGTH = 104, // 0x68
};

#endif // HYBRIS_BINDER_TYPES_H
