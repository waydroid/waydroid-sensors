/*
 * Copyright © 2016 Canonical Ltd.
 * Copyright © 2021 Anbox Project.
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

#include <utils/handler_registration.h>

namespace
{
auto const null_unregister = []{};
}

anbox::core::HandlerRegistration::HandlerRegistration()
    : unregister{null_unregister}
{
}

anbox::core::HandlerRegistration::HandlerRegistration(std::function<void()> const& unregister)
    : unregister{unregister}
{
}

anbox::core::HandlerRegistration::~HandlerRegistration()
{
    unregister();
}

anbox::core::HandlerRegistration::HandlerRegistration(HandlerRegistration&& other)
    : unregister{std::move(other.unregister)}
{
    other.unregister = null_unregister;
}

anbox::core::HandlerRegistration&
anbox::core::HandlerRegistration::operator=(HandlerRegistration&& other)
{
    if (&other != this)
    {
        unregister();
        unregister = std::move(other.unregister);
        other.unregister = null_unregister;
    }

    return *this;
}
