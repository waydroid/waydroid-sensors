/**
   @file socketreader.h
   @brief SocketReader helper class for sensor interface

   <p>
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

#include <QObject>
#include <QLocalSocket>
#include <QVector>

/**
 * @brief Helper class for reading socket datachannel from sensord
 *
 * SocketReader provides common handler for all sensors using socket
 * data channel. It is used by AbstractSensorChannelInterface to maintain
 * the socket connection to the server.
 */
class SocketReader : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SocketReader)

public:

    /**
     * Constructor.
     *
     * @param parent Parent QObject.
     */
    SocketReader(QObject* parent = 0);

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
     * Provides access to the internal QLocalSocket for direct reading.
     *
     * @return Pointer to the internal QLocalSocket. Pointer can be \c NULL
     *         if \c initiateConnection() has not been called successfully.
     */
    QLocalSocket* socket();

    /**
     * Attempt to read given number of bytes from the socket. As
     * QLocalSocket is used, we are guaranteed that any number of bytes
     * written in single operation are available for immediate reading
     * with a single operation.
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
    bool read(QVector<T>& values);

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

    QLocalSocket* socket_; /**< socket data connection to sensord */
    bool tagRead_; /**< is initial magic byte read from the socket */
};

template<typename T>
bool SocketReader::read(QVector<T>& values)
{
    if (!socket_) {
        return false;
    }

    unsigned int count;
    if(!read((void*)&count, sizeof(unsigned int)))
    {
        socket_->readAll();
        return false;
    }
    if(count > 1000)
    {
        qWarning() << "Too many samples waiting in socket. Flushing it to empty";
        socket_->readAll();
        return false;
    }
    values.resize(values.size() + count);
    if(!read((void*)values.data(), sizeof(T) * count))
    {
        qWarning() << "Error occured while reading data from socket: " << socket_->errorString();
        socket_->readAll();
        return false;
    }
    return true;
}
