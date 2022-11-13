/*
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
 * Authored by: Erfan Abdi <erfangplus@gmail.com>
 */

#include "Sensors.h"

using waydroid::sensors::implementation::Sensors;

#define RET_OK          (0)
#define RET_NOTFOUND    (1)
#define RET_INVARG      (2)
#define RET_ERR         (3)

#define DEFAULT_DEVICE  "/dev/anbox-hwbinder"
#define DEFAULT_IFACE   "android.hardware.sensors@1.0::ISensors"
#define DEFAULT_NAME    "default"

typedef struct app {
    GMainLoop* loop;
    GBinderServiceManager* sm;
    GBinderLocalObject* obj;
    int ret;
    Sensors *service;
} App;

typedef struct response {
    GBinderRemoteRequest* req;
    GBinderLocalReply* reply;
    int maxCount;
    Sensors *service;
} Response;

static const char logtag[] = "waydroid-sensors-daemon";

static
gboolean
app_signal(
    gpointer user_data)
{
    App* app = (App*) user_data;

    GINFO("Caught signal, shutting down...");
    app->service->killLoops();
    g_main_loop_quit(app->loop);
    return G_SOURCE_CONTINUE;
}

#define sensors_write_hidl_string_data(writer,ptr,field,index,off) \
     sensors_write_string_with_parent(writer, &ptr->field, index, \
        (off) + ((guint8*)(&ptr->field) - (guint8*)ptr))

static
inline
void
sensors_write_string_with_parent(
    GBinderWriter* writer,
    const GBinderHidlString* str,
    guint32 index,
    guint32 offset)
{
    GBinderParent parent;

    parent.index = index;
    parent.offset = offset;

    /* Strings are NULL-terminated, hence len + 1 */
    gbinder_writer_append_buffer_object_with_parent(writer, str->data.str,
        str->len + 1, &parent);
}

static
void
sensors_write_info_strings(
    GBinderWriter* w,
    const sensor_t* sensor,
    guint idx,
    guint i)
{
    const guint off = sizeof(*sensor) * i;

    /* Write the string data in the right order */
    sensors_write_hidl_string_data(w, sensor, name, idx, off);
    sensors_write_hidl_string_data(w, sensor, vendor, idx, off);
    sensors_write_hidl_string_data(w, sensor, typeAsString, idx, off);
    sensors_write_hidl_string_data(w, sensor, requiredPermission, idx, off);
}

static
gboolean
app_async_resp(
    gpointer user_data)
{
    Response* resp = (Response*)user_data;
    int err = 0;
    GBinderWriter writer;

    std::vector<sensors_event_t> event_vec = resp->service->poll(resp->maxCount, &err);
    sensors_event_t *event = &event_vec[0];
    int event_len = event_vec.size();

    gbinder_local_reply_init_writer(resp->reply, &writer);
    gbinder_writer_append_int32(&writer, err);
    gbinder_writer_append_hidl_vec(&writer, (void *)event, event_len, sizeof(sensors_event_t));

    std::vector<sensor_t> sensors_vec;
    sensor_t *sensors = &sensors_vec[0];
    int sensors_len = sensors_vec.size();
    gbinder_writer_append_hidl_vec(&writer, (void *)sensors, sensors_len, sizeof(sensor_t));

    gbinder_remote_request_complete(resp->req, resp->reply, 0);
    return G_SOURCE_REMOVE;
}

static
void
app_async_free(
    gpointer user_data)
{
    Response* resp = (Response*)user_data;

    gbinder_local_reply_unref(resp->reply);
    gbinder_remote_request_unref(resp->req);
    g_free(resp);
}

static
GBinderLocalReply*
app_reply(
    GBinderLocalObject* obj,
    GBinderRemoteRequest* req,
    guint code,
    guint flags,
    int* status,
    void* user_data)
{
    App* app = (App*) user_data;
    GBinderLocalReply *reply = NULL;
    GBinderReader reader;
    GBinderWriter writer;

    gbinder_remote_request_init_reader(req, &reader);
    if (code == GET_SENSORS_LIST) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            sensor_t* sensors;
            std::vector<sensor_t> sensors_vec = app->service->getSensorsList();
            int sensors_len = sensors_vec.size();

            gbinder_local_reply_init_writer(reply, &writer);

            guint index;
            GBinderParent vec_parent;
            GBinderHidlVec *vec = gbinder_writer_new0(&writer, GBinderHidlVec);
            const gsize total = sensors_len * sizeof(*sensors);
            sensors = (sensor_t*) gbinder_writer_malloc0(&writer, total);

            /* Fill in the vector descriptor */
            if (sensors) {
                vec->data.ptr = sensors;
                vec->count = sensors_len;
            }
            vec->owns_buffer = TRUE;

            std::copy(sensors_vec.begin(), sensors_vec.end(), sensors);

            /* Prepare parent descriptor for the string data */
            vec_parent.index = gbinder_writer_append_buffer_object(&writer, vec, sizeof(*vec));
            vec_parent.offset = GBINDER_HIDL_VEC_BUFFER_OFFSET;

            index = gbinder_writer_append_buffer_object_with_parent(&writer,
                sensors, total, &vec_parent);

            for (int i = 0; i < sensors_len; i++)
                sensors_write_info_strings(&writer, sensors + i, index, i);
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == SET_OPERATION_MODE) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            gint32 tmp = 0;
            gbinder_reader_read_int32(&reader, &tmp);
            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, RESULT_INVALID_OPERATION);
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == ACTIVATE) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            int handle = 0;
            gboolean enabled;
            gbinder_reader_read_int32(&reader, &handle);
            gbinder_reader_read_bool(&reader, &enabled);

            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, app->service->activate(handle, enabled == TRUE));
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == POLL) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            int maxCount = 0;
            gbinder_reader_read_int32(&reader, &maxCount);

            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            Response* resp = g_new0(Response, 1);
            resp->service = app->service;
            resp->maxCount = maxCount;
            resp->reply = reply;
            resp->req = gbinder_remote_request_ref(req);
            g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, app_async_resp,
                                resp, app_async_free);
            gbinder_remote_request_block(resp->req);
            return NULL;
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == BATCH) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            gint32 tmp = 0;
            gint64 tmp64 = 0;
            gbinder_reader_read_int32(&reader, &tmp);
            gbinder_reader_read_int64(&reader, &tmp64);
            gbinder_reader_read_int64(&reader, &tmp64);

            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, RESULT_OK);
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == FLUSH) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            int handle = 0;
            gbinder_reader_read_int32(&reader, &handle);

            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, app->service->flush(handle));
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == INJECT_SENSOR_DATA) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, RESULT_INVALID_OPERATION);
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == REGISTER_DIRECT_CHANNEL) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, RESULT_INVALID_OPERATION);
            gbinder_writer_append_int32(&writer, -1);
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == UNREGISTER_DIRECT_CHANNEL) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            int tmp = 0;
            gbinder_reader_read_int32(&reader, &tmp);

            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, RESULT_OK);
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    } else if (code == CONFIG_DIRECT_REPORT) {
        const char* iface = gbinder_remote_request_interface(req);

        if (!g_strcmp0(iface, DEFAULT_IFACE)) {
            reply = gbinder_local_object_new_reply(obj);

            gbinder_local_reply_append_int32(reply, GBINDER_STATUS_OK);
            *status = GBINDER_STATUS_OK;

            gbinder_local_reply_init_writer(reply, &writer);
            gbinder_writer_append_int32(&writer, RESULT_INVALID_OPERATION);
            gbinder_writer_append_int32(&writer, -1);
        } else {
            GDEBUG("Unexpected interface \"%s\"", iface);
        }
    }

    return reply;
}

static
void
app_add_service_done(
    GBinderServiceManager* sm,
    int status,
    void* user_data)
{
    App* app = (App*) user_data;

    if (status == GBINDER_STATUS_OK) {
        printf("Added \"%s\"\n", DEFAULT_NAME);
        app->ret = RET_OK;
    } else {
        GERR("Failed to add \"%s\" (%d)", DEFAULT_NAME, status);
        g_main_loop_quit(app->loop);
    }
}

static
void
app_sm_presence_handler(
    GBinderServiceManager* sm,
    void* user_data)
{
    App* app = (App*) user_data;

    if (gbinder_servicemanager_is_present(app->sm)) {
        GINFO("Service manager has reappeared");
        gbinder_servicemanager_add_service(app->sm, DEFAULT_NAME, app->obj,
            app_add_service_done, app);
    } else {
        GINFO("Service manager has died");
        app->service->killLoops();
    }
}

static
void
app_run(
   App* app)
{
    const char* name = DEFAULT_NAME;
    guint sigtrm = g_unix_signal_add(SIGTERM, app_signal, app);
    guint sigint = g_unix_signal_add(SIGINT, app_signal, app);
    gulong presence_id = gbinder_servicemanager_add_presence_handler
        (app->sm, app_sm_presence_handler, app);

    app->loop = g_main_loop_new(NULL, TRUE);

    gbinder_servicemanager_add_service(app->sm, DEFAULT_NAME, app->obj,
        app_add_service_done, app);

    GINFO("Waydroid Sensors HAL service ready.");

    g_main_loop_run(app->loop);

    if (sigtrm) g_source_remove(sigtrm);
    if (sigint) g_source_remove(sigint);
    gbinder_servicemanager_remove_handler(app->sm, presence_id);
    g_main_loop_unref(app->loop);
    app->loop = NULL;
}

int main(int argc, char* argv[])
{
    const char* device;
    App app;

    gutil_log_timestamp = FALSE;
    gutil_log_set_type(GLOG_TYPE_STDERR, logtag);
    gutil_log_default.level = GLOG_LEVEL_DEFAULT;

    if (argc < 2)
        device = DEFAULT_DEVICE;
    else
        device = argv[1];

    memset(&app, 0, sizeof(app));
    app.ret = RET_INVARG;
    app.service = new Sensors();

    app.sm = gbinder_servicemanager_new2(device, "hidl", "hidl");
    if (gbinder_servicemanager_wait(app.sm, -1)) {
        app.obj = gbinder_servicemanager_new_local_object
            (app.sm, DEFAULT_IFACE, app_reply, &app);
        app_run(&app);
        gbinder_local_object_unref(app.obj);
        gbinder_servicemanager_unref(app.sm);
    }
    return app.ret;
}
