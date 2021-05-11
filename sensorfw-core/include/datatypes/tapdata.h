/**
   @file tapdata.h
   @brief Datatype for device tap events

   <p>
   Copyright (C) 2009-2010 Nokia Corporation

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

#ifndef TAPDATA_H
#define TAPDATA_H

#include <datatypes/genericdata.h>

/**
 * @brief Datatype for device tap events.
 *
 * Contains enumerated values for different types of tap events.
 */
class TapData : public TimedData {
public:
    /**
     * Direction of tap. The last six directions may not be supported
     * depending on hardware.
     */
    enum Direction
    {
        X = 0,     /**< Left or right side tapped */
        Y,         /**< Top or down side tapped */
        Z,         /**< Face or bottom tapped */
        LeftRight, /**< Tapped from left to right */
        RightLeft, /**< Tapped from right to left */
        TopBottom, /**< Tapped from top to bottom */
        BottomTop, /**< Tapped from bottom to top */
        FaceBack,  /**< Tapped from face to back */
        BackFace   /**< Tapped from back to face */
    };

    /**
     * Type of tap.
     */
    enum Type
    {
        DoubleTap = 0, /**< Double tap. */
        SingleTap      /**< Single tap. */
    };

    TapData::Direction direction_; /**< Direction of tap */
    TapData::Type type_;           /**< Type of tap */

    /**
     * Constructor.
     */
    TapData() : TimedData(0), direction_(X), type_(SingleTap) {}

    /**
     * Constructor.
     * @param timestamp Timestamp of tap event.
     * @param direction Direction of tap.
     * @param type Type of tap.
     */
    TapData(const quint64& timestamp, Direction direction, Type type) :
        TimedData(timestamp), direction_(direction), type_(type) {}
};

#endif // TAPDATA_H
