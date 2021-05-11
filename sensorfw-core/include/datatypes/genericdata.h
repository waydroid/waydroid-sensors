/**
   @file genericdata.h
   @brief Basic datatypes for filters

   <p>
   Copyright (C) 2009-2010 Nokia Corporation

   @author Joep van Gassel <joep.van.gassel@nokia.com>
   @author Timo Rongas <ext-timo.2.rongas@nokia.com>

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

#ifndef GENERICDATA_H
#define GENERICDATA_H

#include <QMetaType>

/**
 * A base class for measurement data that contain timestamp.
 */
class TimedData
{
public:

    /**
     * Constructor
     *
     * @param timestamp monotonic time (microsec)
     */
    TimedData(const quint64& timestamp) : timestamp_(timestamp) {}

    quint64 timestamp_;  /**< monotonic time (microsec) */
};

/**
 * Class for vector type measurement data (timestamp, x, y, z).
 */
class TimedXyzData : public TimedData
{
public:
    /**
     * Constructor.
     */
    TimedXyzData() : TimedData(0), x_(0), y_(0), z_(0) {}

    /**
     * Constructor.
     *
     * @param timestamp monotonic time (microsec)
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     */
    TimedXyzData(const quint64& timestamp, int x, int y, int z) : TimedData(timestamp), x_(x), y_(y), z_(z) {}

    int x_; /**< X value */
    int y_; /**< Y value */
    int z_; /**< Z value */
};
Q_DECLARE_METATYPE ( TimedXyzData )

#endif // GENERICDATA_H
