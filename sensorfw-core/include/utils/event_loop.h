/*
 * Copyright © 2016 Canonical Ltd.
 * Copyright © 2021 Waydroid Project.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#pragma once

#include <thread>
#include <functional>
#include <future>
#include <string>

#include <glib.h>

namespace waydroid
{
namespace core
{

using EventLoopCancellation = std::function<void()>;

class EventLoop
{
public:
    EventLoop(std::string const& name);
    ~EventLoop();

    void stop();

    std::future<void> enqueue(std::function<void()> const& callback);
    std::future<void> schedule_in(
        std::chrono::milliseconds, std::function<void()> const& callback);

    void schedule_with_cancellation_in(
        std::chrono::milliseconds,
        std::function<void()> const& callback,
        std::function<void(EventLoopCancellation const&)> const& cancellation_ready);

    void watch_fd(int fd, std::function<void()> const& callback);

protected:
    std::thread loop_thread;
    GMainContext* main_context;
    GMainLoop* main_loop;
};

}
}
