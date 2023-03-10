/**
   @file socketreader.cpp
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

#include <utils/socketreader.h>

#include <memory>
#include <string>

#include <glib.h>
#include <gio/gio.h>
#include <gio/ginputstream.h>
#include <gio/giostream.h>
#include <gio/goutputstream.h>
#include <gio/gsocketclient.h>
#include <gio/gsocketconnectable.h>
#include <gio/gsocketconnection.h>
#include <gio/gunixsocketaddress.h>

const char* SocketReader::channelIDString = "_SENSORCHANNEL_";

SocketReader::SocketReader() :
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
        g_debug("attempting to initiate connection on connected socket");
        return false;
    }

    const std::string SOCKET_NAME {"/var/run/sensord.sock"};
    const char* env = getenv("SENSORFW_SOCKET_PATH");
    auto full_path = env ? env : "" + SOCKET_NAME;
    auto sock_addr = std::unique_ptr<GSocketAddress, decltype(&g_object_unref)>{
        g_unix_socket_address_new(full_path.c_str()), g_object_unref};

    auto sock_client = std::unique_ptr<GSocketClient, decltype(&g_object_unref)>{
        g_socket_client_new(), g_object_unref};

    g_autoptr(GError) err = NULL;
    socket_ = g_socket_client_connect(
        sock_client.get(),
        G_SOCKET_CONNECTABLE(sock_addr.get()),
        /* cancellable */ NULL,
        &err);

    if (!socket_) {
        g_debug("Failed to connect to socket: %s", err->message);
        return false;
    }

    istream_ = g_io_stream_get_input_stream(G_IO_STREAM(socket_));
    ostream_ = g_io_stream_get_output_stream(G_IO_STREAM(socket_));

    if (!g_output_stream_write_all(
            ostream_,
            (const char*)&sessionId,
            sizeof(sessionId),
            /* (out) bytes_written */ NULL,
            /* cancellable */ NULL,
            &err)) {
        g_debug("[SOCKETREADER]: SessionId write failed: %s", err->message);
        g_clear_error(&err);
    }

    if (!g_output_stream_flush(ostream_, /* cancellable */ NULL, &err)) {
        g_debug("[SOCKETREADER]: flush write failed: %s", err->message);
        g_clear_error(&err);
    }

    readSocketTag();

    return true;
}

bool SocketReader::dropConnection()
{
    if (!socket_)
        return false;

    /* These are owned by socket */
    istream_ = NULL;
    ostream_ = NULL;

    g_io_stream_close(
        G_IO_STREAM(socket_),
        /* cancellable */ NULL,
        /* (out) err */ NULL);
    g_object_unref(socket_);
    socket_ = NULL;

    tagRead_ = false;

    return true;
}

GSocketConnection* SocketReader::socket()
{
    return socket_;
}

bool SocketReader::readSocketTag()
{   
    tagRead_ = g_input_stream_skip(
        istream_, /* count */ 1,
        /* cancellable */ NULL,
        /* (out) error */ NULL);
    return true;
}

bool SocketReader::read(void* buffer, int size)
{
    int bytesRead = 0;

    while(bytesRead < size)
    {
        g_autoptr(GError) err = NULL;

        int bytes = g_input_stream_read(
            istream_,
            (char *)buffer + bytesRead,
            size - bytesRead,
            /* cancellable */ NULL,
            /* (out) err */ &err);

        if (err) {
            g_warning("Failed to read from socket: %s. Unexpected things might occur.", err->message);
            return false;
        }

        if(bytes == 0)
        {
            // This is EOF. No further reading possible.
            break;
        }

        if(bytes < 1)
            return false;
        bytesRead += bytes;
    }
    return (bytesRead > 0);
}

bool SocketReader::isConnected()
{
    return (
        socket_ &&
        g_socket_connection_is_connected(socket_) && 
        !g_io_stream_is_closed(G_IO_STREAM(socket_)));
}

void SocketReader::skipAll()
{
    while (g_pollable_input_stream_is_readable(G_POLLABLE_INPUT_STREAM(istream_))) {
        g_autoptr(GError) err = NULL;

        auto ret = g_input_stream_skip(
            istream_,
            /* count */ G_MAXSSIZE,
            /* cancellable */ NULL,
            /* (out) error */ &err);

        if (err) {
            g_warning("Cannot skip the stream: %s. Unexpected things might occur.", err->message);
            break;
        }

        if (ret == 0) {
            // This is EOF.
            break;
        }
    }
}
