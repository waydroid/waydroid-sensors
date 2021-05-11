/**
   @file posedata.h
   @brief Datatype for device 'pose' (orientation)

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

#ifndef POSEDATA_H
#define POSEDATA_H

#include <datatypes/genericdata.h>

/**
 * Datatype for device pose interpretation.
 */
class PoseData : public TimedData {
public:
    /**
     * Possible device postures.
     * @note The interpretation algorithm for orientation currently relies on the
     *       integer values of the enumeration. Thus changing the names for the
     *       orientation states is completely ok (for sensord, client apps may
     *       disagree). Possible new values must be appended to the list and the
     *       order of values must not be changed!
     *
     * Device side naming:
     * @verbatim

                          Top


                      ----------
                     /  NOKIA  /|
                    /-------- / |
                   //       //  /
                  //       //  /
          Left   //  Face //  /    Right
                //       //  /
               //       //  /
              /---------/  /
             /    O    /  /
            /         /  /
            ----------  /
            |_________!/


              Bottom

       @endverbatim
     */

    /**
     * Device orientation.
     */
    enum Orientation
    {
        Undefined = 0, /**< Orientation is unknown. */
        LeftUp,        /**< Device left side is up */
        RightUp,       /**< Device right side is up */
        BottomUp,      /**< Device bottom is up */
        BottomDown,    /**< Device bottom is down */
        FaceDown,      /**< Device face is down */
        FaceUp         /**< Device face is up */
    };

    PoseData::Orientation orientation_; /**< Device Orientation */

    /**
     * Constructor.
     */
    PoseData() : TimedData(0), orientation_(Undefined) {}

    /**
     * Constructor.
     * @param orientation Initial value for orientation.
     */
    PoseData(Orientation orientation) : TimedData(0), orientation_(orientation) {}

    /**
     * Constructor
     * @param timestamp Initial value for timestamp.
     * @param orientation Initial value for orientation.
     */
    PoseData(const quint64& timestamp, Orientation orientation) : TimedData(timestamp), orientation_(orientation) {}
};

Q_DECLARE_METATYPE(PoseData)

#endif // POSEDATA_H
