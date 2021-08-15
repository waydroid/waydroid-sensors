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

#include <utils/handler_registration.h>
#include <utils/event_loop.h>

namespace waydroid
{
namespace core
{

class EventLoopHandlerRegistration : public HandlerRegistration
{
public:
    EventLoopHandlerRegistration(
        waydroid::core::EventLoop& loop,
        std::function<void()> const& register_func,
        std::function<void()> const& unregister)
        : HandlerRegistration{[&, unregister] { loop.enqueue(unregister).wait(); }}
    {
        loop.enqueue(register_func).wait();
    }

    EventLoopHandlerRegistration(
        waydroid::core::EventLoop& loop,
        std::function<void()> const& unregister)
        : HandlerRegistration{[&, unregister] { loop.enqueue(unregister).wait(); }}
    {
    }
};

}
}
