/**
   @file socketreader.cpp
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

#include <utils/socketreader.h>

const char* SocketReader::channelIDString = "_SENSORCHANNEL_";

SocketReader::SocketReader(QObject* parent) :
    QObject(parent),
    socket_(NULL),
    tagRead_(false)
{
}

SocketReader::~SocketReader()
{
    if (socket_) {
        dropConnection();
    }
}

bool SocketReader::initiateConnection(int sessionId)
{
    if (socket_ != NULL) {
        qDebug() << "attempting to initiate connection on connected socket";
        return false;
    }

    socket_ = new QLocalSocket(this);
    const char* SOCKET_NAME = "/var/run/sensord.sock";
    QByteArray env = qgetenv("SENSORFW_SOCKET_PATH");
    if (!env.isEmpty()) {
        env += SOCKET_NAME;
        SOCKET_NAME = env;
    }

    socket_->connectToServer(SOCKET_NAME, QIODevice::ReadWrite);

    if (!(socket_->serverName().size())) {
        qDebug() << socket_->errorString();
        return false;
    }

    if (socket_->write((const char*)&sessionId, sizeof(sessionId)) != sizeof(sessionId)) {
        qDebug() << "[SOCKETREADER]: SessionId write failed: " << socket_->errorString();
    }
    socket_->flush();
    readSocketTag();

    return true;
}

bool SocketReader::dropConnection()
{
    if (!socket_)
        return false;

    socket_->disconnectFromServer();
    if(socket_->state() != QLocalSocket::UnconnectedState)
        socket_->waitForDisconnected();
    delete socket_;
    socket_ = NULL;

    tagRead_ = false;

    return true;
}

QLocalSocket* SocketReader::socket()
{
    return socket_;
}

bool SocketReader::readSocketTag()
{
    char foo;
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    socket_->waitForReadyRead();
    tagRead_ = read(&foo, 1);
    return true;
}

bool SocketReader::read(void* buffer, int size)
{
    int bytesRead = 0;
    int retry = 100;
    while(bytesRead < size)
    {
        int bytes = socket_->read((char *)buffer + bytesRead, size);
        if(bytes == 0)
        {
            if(!retry)
                return false;
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 100000000;
            nanosleep( &ts, NULL );
            --retry;
            continue;
        }
        if(bytes < 1)
            return false;
        bytesRead += bytes;
    }
    return (bytesRead > 0);
}

bool SocketReader::isConnected()
{
    return (socket_ && socket_->isValid() && socket_->state() == QLocalSocket::ConnectedState);
}
