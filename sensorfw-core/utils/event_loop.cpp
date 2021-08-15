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

#include <utils/event_loop.h>

#include <glib-unix.h>
#include <pthread.h>

namespace
{

void set_thread_name(std::thread& thread, std::string const& name)
{
    static size_t const max_name_len = 15;
    auto const proper_name = name.substr(0, max_name_len);

    pthread_setname_np(thread.native_handle(), proper_name.c_str());
}

struct GSourceContext
{
    GSourceContext(std::function<void()> const& callback)
        : callback{callback}
    {
    }

    static gboolean static_call(GSourceContext* ctx)
    {
        try
        {
            ctx->callback();
            ctx->done.set_value();
        }
        catch (...)
        {
            ctx->done.set_exception(std::current_exception());
        }
        return G_SOURCE_REMOVE;
    }

    static void static_destroy(GSourceContext* ctx) { delete ctx; }
    std::function<void()> const callback;
    std::promise<void> done;
};

struct GSourceFdContext
{
    GSourceFdContext(std::function<void()> const& callback)
        : callback{callback}
    {
    }

    static gboolean static_call(int, GIOCondition, GSourceFdContext* ctx)
    {
        try
        {
            ctx->callback();
        }
        catch (...)
        {
        }
        return G_SOURCE_CONTINUE;
    }

    static void static_destroy(GSourceFdContext* ctx) { delete ctx; }
    std::function<void()> const callback;
};

}

waydroid::core::EventLoop::EventLoop(std::string const& name)
    : main_context{g_main_context_new()},
      main_loop{g_main_loop_new(main_context, FALSE)}
{
    loop_thread = std::thread{
        [this]
        {
            g_main_context_push_thread_default(main_context);
            g_main_loop_run(main_loop);
        }};

    set_thread_name(loop_thread, name);

    enqueue([]{}).wait();
}

waydroid::core::EventLoop::~EventLoop()
{
    stop();
}

void waydroid::core::EventLoop::stop()
{
    if (main_loop)
        g_main_loop_quit(main_loop);
    if (loop_thread.joinable())
        loop_thread.join();
    if (main_loop)
    {
        g_main_loop_unref(main_loop);
        main_loop = nullptr;
    }
    if (main_context)
    {
        g_main_context_unref(main_context);
        main_context = nullptr;
    }
}

std::future<void> waydroid::core::EventLoop::enqueue(std::function<void()> const& callback)
{
    auto const gsource = g_idle_source_new();
    auto const ctx = new GSourceContext{callback};
    g_source_set_callback(
            gsource,
            reinterpret_cast<GSourceFunc>(&GSourceContext::static_call),
            ctx,
            reinterpret_cast<GDestroyNotify>(&GSourceContext::static_destroy));

    auto future = ctx->done.get_future();

    g_source_attach(gsource, main_context);
    g_source_unref(gsource);

    return future;
}

std::future<void> waydroid::core::EventLoop::schedule_in(
    std::chrono::milliseconds timeout,
    std::function<void()> const& callback)
{
    auto const gsource = g_timeout_source_new(timeout.count());
    auto const ctx = new GSourceContext{callback};
    g_source_set_callback(
            gsource,
            reinterpret_cast<GSourceFunc>(&GSourceContext::static_call),
            ctx,
            reinterpret_cast<GDestroyNotify>(&GSourceContext::static_destroy));

    auto future = ctx->done.get_future();

    g_source_attach(gsource, main_context);
    g_source_unref(gsource);

    return future;
}

void waydroid::core::EventLoop::schedule_with_cancellation_in(
    std::chrono::milliseconds timeout,
    std::function<void()> const& callback,
    std::function<void(EventLoopCancellation const&)> const& cancellation_ready)
{
    auto const gsource = g_timeout_source_new(timeout.count());
    auto const ctx = new GSourceContext{callback};
    g_source_set_callback(
            gsource,
            reinterpret_cast<GSourceFunc>(&GSourceContext::static_call),
            ctx,
            reinterpret_cast<GDestroyNotify>(&GSourceContext::static_destroy));

    auto const cancellation =
        [this, gsource]
        {
            g_source_destroy(gsource);
            g_source_unref(gsource);
        };

    enqueue(
        [cancellation, cancellation_ready]
        {
            cancellation_ready(cancellation);
        });

    g_source_attach(gsource, main_context);
}

void waydroid::core::EventLoop::watch_fd(
    int fd, std::function<void()> const& callback)
{
    auto const gsource = g_unix_fd_source_new(fd, G_IO_IN);
    auto const ctx = new GSourceFdContext{callback};
    g_source_set_callback(
            gsource,
            reinterpret_cast<GSourceFunc>(&GSourceFdContext::static_call),
            ctx,
            reinterpret_cast<GDestroyNotify>(&GSourceFdContext::static_destroy));

    g_source_attach(gsource, main_context);
    g_source_unref(gsource);
}
