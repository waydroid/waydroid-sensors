/**
   @file orientationdata.h
   @brief Datatypes for different filters

   <p>
   Copyright (C) 2009-2010 Nokia Corporation

   @author Joep van Gassel <joep.van.gassel@nokia.com>
   @author Timo Rongas <ext-timo.2.rongas@nokia.com>
   @author Ustun Ergenoglu <ext-ustun.ergenoglu@nokia.com>
   @author Antti Virtanen <antti.i.virtanen@nokia.com>

   This file is part of Sensord.

   Sensord is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   Sensord is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with Sensord.  If not, see <http://www.gnu.org/licenses/>.
   </p>
 */

#ifndef ORIENTATIONDATA_H
#define ORIENTATIONDATA_H

#include <datatypes/genericdata.h>
#include <datatypes/timedunsigned.h>

/**
 * Accelerometer mesurement data.
 */
typedef TimedXyzData AccelerationData;

/**
 * Magnetometer measurement data.
 */
typedef TimedXyzData MagneticFieldData;

/**
 * Device orientation measurement data.
 */
typedef TimedXyzData OrientationData;

/**
 * Magnetometer measurement data.
 */
typedef TimedXyzData MagnetometerData;

/**
 * Datatype for calibrated magnetometer measurements.
 */
class CalibratedMagneticFieldData : public TimedData
{
public:
    /**
     * Default constructor.
     */
    CalibratedMagneticFieldData() : TimedData(0),
                                    x_(0), y_(0), z_(0),
                                    rx_(0), ry_(0), rz_(0),
                                    level_(0) {}

    /**
     * Constructor.
     *
     * @param timestamp timestamp as monotonic time (microsec).
     * @param x X coordinate value.
     * @param y Y coordinate value.
     * @param z Z coordinate value.
     * @param rx raw X coordinate value.
     * @param ry raw Y coordinate value.
     * @param rz raw Z coordinate value.
     * @param level Calibration level.
     */
    CalibratedMagneticFieldData(const uint64_t& timestamp, int x, int y, int z, int rx, int ry, int rz, int level) :
        TimedData(timestamp),
        x_(x), y_(y), z_(z),
        rx_(rx), ry_(ry), rz_(rz),
        level_(level) {}

    /**
     * Constructor.
     *
     * @param magData Magnetometer data.
     * @param level Calibration level.
     */
    CalibratedMagneticFieldData(TimedXyzData magData, int level) :
        TimedData(magData.timestamp_),
        x_(0), y_(0), z_(0),
        rx_(magData.x_), ry_(magData.y_), rz_(magData.z_),
        level_(level) {}

    int x_;     /**< X coordinate value */
    int y_;     /**< Y coordinate value */
    int z_;     /**< Z coordinate value */
    int rx_;    /**< raw X coordinate value */
    int ry_;    /**< raw Y coordinate value */
    int rz_;    /**< raw Z coordinate value */
    int level_; /**< Magnetometer calibration level. Higher value means better calibration. */
};

/**
 * Datatype for compass measurements.
 */
class CompassData : public TimedData
{
public:
    /**
     * Default constructor.
     */
    CompassData() : TimedData(0), degrees_(0), rawDegrees_(0), correctedDegrees_(0), level_(0) {}

    /**
     * Constructor.
     *
     * @param timestamp timestamp as monotonic time (microsec).
     * @param degrees Angle to north.
     * @param level Magnetometer calibration level.
     */
    CompassData(const uint64_t& timestamp, int degrees, int level) :
        TimedData(timestamp), degrees_(degrees), rawDegrees_(degrees), correctedDegrees_(0), level_(level) {}

    /**
     * Constructor.
     *
     * @param timestamp timestamp as monotonic time (microsec).
     * @param degrees Angle to north.
     * @param level Magnetometer calibration level.
     * @param correctedDegrees Declination corrected angle to north.
     * @param rawDegrees Not declination corrected angle to north.
     */
    CompassData(const uint64_t& timestamp, int degrees, int level, int correctedDegrees, int rawDegrees) :
        TimedData(timestamp), degrees_(degrees), rawDegrees_(rawDegrees), correctedDegrees_(correctedDegrees), level_(level) {}

    int degrees_; /**< Angle to north which may be declination corrected or not. This is the value apps should use */
    int rawDegrees_; /**< Angle to north without declination correction */
    int correctedDegrees_; /**< Declination corrected angle to north */
    int level_;   /**< Magnetometer calibration level. Higher value means better calibration. */
};

/**
 * Datatype for proximity measurements
 */
class ProximityData : public TimedUnsigned
{
public:
    /**
     * Default constructor.
     */
    ProximityData() : TimedUnsigned(), withinProximity_(false) {}

    /**
     * Constructor
     *
     * @param timestamp timestamp as monotonic time (microsec).
     * @param value raw proximity value.
     * @param withinProximity is there an object within proximity.
     */
    ProximityData(const uint64_t& timestamp, unsigned int value, bool withinProximity) :
        TimedUnsigned(timestamp, value), withinProximity_(withinProximity) {}

    bool withinProximity_; /**< is an object within proximity or not */
};

#endif // ORIENTATIONDATA_H
