/**
   @file liddata.h
   @brief Datatype for device tap events

   <p>
   Copyright (C) 2016 Canonical,  Ltd.

   @author Lorn Potter <lorn.potter@canonical.com>

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

#ifndef LIDDATA_H
#define LIDDATA_H

#include <datatypes/genericdata.h>

/**
 * @brief Datatype for device lid events.
 *
 * Contains enumerated values for different types of lid events.
 */
class LidData : public TimedData {
public:

    /**
     * Type of lid.
     */
    enum Type {
        UnknownLid = -1,
        FrontLid = 0, /**< Front lid. */
        BackLid      /**< Back lid. */
    };

    LidData::Type type_; /**< Type of lid */
    unsigned value_; /**< Measurement value. */

    /**
     * Constructor.
     */
    LidData() : TimedData(0), type_(FrontLid), value_(0) {}

    /**
     * Constructor.
     * @param timestamp Timestamp of lid event.
     * @param type Type of lid.
     * @param value Initial value to use
     */
    LidData(const uint64_t& timestamp, Type type, unsigned value) :
        TimedData(timestamp), type_(type), value_(value) {}
};

#endif // LidData_H
