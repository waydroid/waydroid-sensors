/**
   @file socketreader.h
   @brief SocketReader helper class for sensor interface

   <p>
   Copyright 2022 UBports Foundation.
   Copyright (C) 2009-2010 Nokia Corporation

   @author Timo Rongas <ext-timo.2.rongas@nokia.com>
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

#pragma once

#include <vector>

#include <glib.h>
#include <gio/gio.h>

/**
 * @brief Helper class for reading socket datachannel from sensord
 *
 * SocketReader provides common handler for all sensors using socket
 * data channel. It is used by AbstractSensorChannelInterface to maintain
 * the socket connection to the server.
 */
class SocketReader
{
public:

    /**
     * Constructor.
     */
    SocketReader();

    /**
     * Destructor.
     */
    ~SocketReader();

    /**
     * Initiates new data socket connection.
     *
     * @param sessionId ID for the current session.
     * @return was the connection established successfully.
     */
    bool initiateConnection(int sessionId);

    /**
     * Drops socket connection.
     * @return was the connection successfully closed.
     */
    bool dropConnection();

    /**
     * Provides access to the internal GSocketConnection for direct reading.
     *
     * @return Pointer to the internal GSocketConnection. Pointer can be \c NULL
     *         if \c initiateConnection() has not been called successfully.
     */
    GSocketConnection* socket();

    /**
     * Attempt to read given number of bytes from the socket.
     *
     * @param size Number of bytes to read.
     * @param buffer Location for storing the data.
     * @return was given amount of bytes read succesfully.
     */
    bool read(void* buffer, int size);

    /**
     * Attempt to read objects from the sockets. The call blocks until
     * there are minimum amount of expected bytes availabled in the socket.
     *
     * @param values Vector to which objects will be appended.
     * @tparam T type of expected object in the stream.
     * @return true if atleast one object was read.
     */
    template<typename T>
    bool read(std::vector<T>& values);

    /**
     * Returns whether the socket is currently connected.
     *
     * @return is socket connected.
     */
    bool isConnected();

private:
    /**
     * Prefix text needed to be written to the sensor daemon socket connection
     * when establishing new session.
     */
    static const char* channelIDString;

    /**
     * Reads initial magic byte from the fresh connection.
     */
    bool readSocketTag();

    /**
     * Skip any readable content on the socket.
     */
    void skipAll();

    GSocketConnection* socket_; /**< socket data connection to sensord */
    GInputStream* istream_; /**< input of socket. owned by socket. */
    GOutputStream* ostream_; /**< output of socket. owned by socket. */
    bool tagRead_; /**< is initial magic byte read from the socket */
};

template<typename T>
bool SocketReader::read(std::vector<T>& values)
{
    if (!socket_) {
        return false;
    }

    unsigned int count;
    if(!read((void*)&count, sizeof(unsigned int)))
    {
        skipAll();
        return false;
    }
    if(count > 1000)
    {
        g_warning("Too many samples waiting in socket. Flushing it to empty");
        skipAll();
        return false;
    }
    values.resize(values.size() + count);
    if(!read((void*)values.data(), sizeof(T) * count))
    {
        g_warning("Error occured while reading data from socket");
        skipAll();
        return false;
    }
    return true;
}
