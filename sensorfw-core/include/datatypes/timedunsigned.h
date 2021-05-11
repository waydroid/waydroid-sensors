/**
   @file timedunsigned.h
   @brief Datatype for unsigned values

   <p>
   Copyright (C) 2009-2010 Nokia Corporation

   @author Joep van Gassel <joep.van.gassel@nokia.com>

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

#ifndef TIMED_UNSIGNED_H
#define TIMED_UNSIGNED_H

#include <datatypes/genericdata.h>

/**
 * @brief Datatype for unsigned integer value with timestamp.
 */
class TimedUnsigned : public TimedData {
public:
    /**
     * Default constructor.
     */
    TimedUnsigned() : TimedData(0), value_(0) {}

    /**
     * Constructor.
     *
     * @param timestamp timestamp as monotonic time (microsec).
     * @param value value of the measurement.
     */
    TimedUnsigned(const quint64& timestamp, unsigned value) : TimedData(timestamp), value_(value) {}

    unsigned value_; /**< Measurement value. */
};

Q_DECLARE_METATYPE ( TimedUnsigned )

#endif // TIMED_UNSIGNED_H
