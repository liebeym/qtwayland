/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "wltouch.h"
#include "wlsurface.h"
#include <QTouchEvent>
#include <QWindow>

namespace Wayland {

static void dummy(wl_client *, wl_resource *)
{
}

const struct wl_touch_extension_interface TouchExtensionGlobal::touch_interface = {
    dummy
};

static const int maxRawPos = 24;

TouchExtensionGlobal::TouchExtensionGlobal(Compositor *compositor)
    : m_compositor(compositor),
      m_flags(0)
{
    wl_array_init(&m_rawdata_array);
    m_rawdata_ptr = static_cast<float *>(wl_array_add(&m_rawdata_array, maxRawPos * sizeof(float) * 2));

    wl_display_add_global(compositor->wl_display(),
                          &wl_touch_extension_interface,
                          this,
                          TouchExtensionGlobal::bind_func);
}

TouchExtensionGlobal::~TouchExtensionGlobal()
{
    wl_array_release(&m_rawdata_array);
}

void TouchExtensionGlobal::destroy_resource(wl_resource *resource)
{
    TouchExtensionGlobal *self = static_cast<TouchExtensionGlobal *>(resource->data);
    self->m_resources.removeOne(resource);
    free(resource);
}

void TouchExtensionGlobal::bind_func(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_resource *resource = wl_client_add_object(client, &wl_touch_extension_interface, &touch_interface, id, data);
    resource->destroy = destroy_resource;
    TouchExtensionGlobal *self = static_cast<TouchExtensionGlobal *>(resource->data);
    self->m_resources.append(resource);
    wl_resource_post_event(resource, WL_TOUCH_EXTENSION_CONFIGURE, self->m_flags);
}

static inline int toFixed(qreal f)
{
    return int(f * 10000);
}

void TouchExtensionGlobal::postTouchEvent(QTouchEvent *event, Surface *surface)
{
    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    const int pointCount = points.count();
    if (!pointCount)
        return;

    QPointF surfacePos = surface->pos();
    wl_client *surfaceClient = surface->base()->resource.client;
    uint32_t time = m_compositor->currentTimeMsecs();
    const int rescount = m_resources.count();

    for (int res = 0; res < rescount; ++res) {
        wl_resource *target = m_resources.at(res);
        if (target->client != surfaceClient)
            continue;

        // We will use no touch_frame type of event, to reduce the number of
        // events flowing through the wire. Instead, the number of points sent is
        // included in the touch point events.
        int sentPointCount = 0;
        for (int i = 0; i < pointCount; ++i) {
            if (points.at(i).state() != Qt::TouchPointStationary)
                ++sentPointCount;
        }

        for (int i = 0; i < pointCount; ++i) {
            const QTouchEvent::TouchPoint &tp(points.at(i));
            // Stationary points are never sent. They are cached on client side.
            if (tp.state() == Qt::TouchPointStationary)
                continue;

            uint32_t id = tp.id();
            uint32_t state = (tp.state() & 0xFFFF) | (sentPointCount << 16);
            uint32_t flags = tp.flags();

            QPointF p = tp.pos() - surfacePos; // surface-relative
            int x = toFixed(p.x());
            int y = toFixed(p.y());
            int nx = toFixed(tp.normalizedPos().x());
            int ny = toFixed(tp.normalizedPos().y());
            int w = toFixed(tp.rect().width());
            int h = toFixed(tp.rect().height());
            int vx = toFixed(tp.velocity().x());
            int vy = toFixed(tp.velocity().y());
            uint32_t pressure = uint32_t(tp.pressure() * 255);

            wl_array *rawData = 0;
            QVector<QPointF> rawPosList = tp.rawScreenPositions();
            int rawPosCount = rawPosList.count();
            if (rawPosCount) {
                rawPosCount = qMin(maxRawPos, rawPosCount);
                rawData = &m_rawdata_array;
                rawData->size = rawPosCount * sizeof(float) * 2;
                float *p = m_rawdata_ptr;
                for (int rpi = 0; rpi < rawPosCount; ++rpi) {
                    const QPointF &rawPos(rawPosList.at(rpi));
                    // This will stay in screen coordinates for performance
                    // reasons, clients using this data will presumably know
                    // what they are doing.
                    *p++ = float(rawPos.x());
                    *p++ = float(rawPos.y());
                }
            }

            wl_resource_post_event(target, WL_TOUCH_EXTENSION_TOUCH,
                                   time, id, state,
                                   x, y, nx, ny, w, h,
                                   pressure, vx, vy,
                                   flags, rawData);
        }
    }
}

}
