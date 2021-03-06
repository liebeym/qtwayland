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

#include "wlcompositor.h"

#include "waylandinput.h"
#include "wldisplay.h"
#include "wlshmbuffer.h"
#include "wlsurface.h"
#include "waylandcompositor.h"
#include "wldatadevicemanager.h"
#include "wldatadevice.h"
#include "wlextendedoutput.h"
#include "wlextendedsurface.h"
#include "wlsubsurface.h"
#include "wlshellsurface.h"
#include "wltouch.h"
#include "wlinputdevice.h"

#include <QWindow>
#include <QSocketNotifier>
#include <QScreen>
#include <QPlatformScreen>
#include <QGuiApplication>
#include <QPlatformScreenPageFlipper>
#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>

#include <wayland-server.h>

#include "hardware_integration/graphicshardwareintegration.h"
#include "waylandwindowmanagerintegration.h"

namespace Wayland {

static Compositor *compositor;

void compositor_create_surface(struct wl_client *client,
                               struct wl_resource *resource, uint32_t id)
{
     static_cast<Compositor *>(resource->data)->createSurface(client,id);
}

const static struct wl_compositor_interface compositor_interface = {
    compositor_create_surface
};

void Compositor::bind_func(struct wl_client *client, void *data,
                      uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    wl_client_add_object(client,&wl_compositor_interface, &compositor_interface, id,data);
}


Compositor *Compositor::instance()
{
    return compositor;
}

Compositor::Compositor(WaylandCompositor *qt_compositor)
    : m_display(new Display)
    , m_default_input_device(0)
    , m_pageFlipper(0)
    , m_shm(m_display)
    , m_current_frame(0)
    , m_last_queued_buf(-1)
    , m_qt_compositor(qt_compositor)
    , m_orientation(Qt::PrimaryOrientation)
    , m_directRenderSurface(0)
#if defined (QT_COMPOSITOR_WAYLAND_GL)
    , m_graphics_hw_integration(0)
#endif
    , m_outputExtension(0)
    , m_surfaceExtension(0)
    , m_subSurfaceExtension(0)
    , m_touchExtension(0)
    , m_retainNotify(0)
{
    compositor = this;
    qDebug() << "Compositor instance is" << this;

#if defined (QT_COMPOSITOR_WAYLAND_GL)
    QWindow *window = qt_compositor->window();
    if (window && window->surfaceType() != QWindow::RasterSurface)
        m_graphics_hw_integration = GraphicsHardwareIntegration::createGraphicsHardwareIntegration(qt_compositor);
#endif
    m_windowManagerIntegration = new WindowManagerServerIntegration(this);

    wl_display_add_global(m_display->handle(),&wl_compositor_interface,this,Compositor::bind_func);

    m_data_device_manager =  new DataDeviceManager(this);

    wl_display_add_global(m_display->handle(),&wl_output_interface, &m_output_global,OutputGlobal::output_bind_func);

    m_shell = new Shell();
    wl_display_add_global(m_display->handle(), &wl_shell_interface, m_shell, Shell::bind_func);

    m_outputExtension = new OutputExtensionGlobal(this);
    m_surfaceExtension = new SurfaceExtensionGlobal(this);

    if (wl_display_add_socket(m_display->handle(), qt_compositor->socketName())) {
        fprintf(stderr, "Fatal: Failed to open server socket\n");
        exit(EXIT_FAILURE);
    }

    m_loop = wl_display_get_event_loop(m_display->handle());

    int fd = wl_event_loop_get_fd(m_loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(sockNot, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));

    qRegisterMetaType<SurfaceBuffer*>("SurfaceBuffer*");
    //initialize distancefieldglyphcache here
}

Compositor::~Compositor()
{
    delete m_shell;
    delete m_outputExtension;
    delete m_surfaceExtension;
    delete m_subSurfaceExtension;
    delete m_touchExtension;

    delete m_default_wayland_input_device;
    delete m_data_device_manager;

#ifdef QT_COMPOSITOR_WAYLAND_GL
    delete m_graphics_hw_integration;
#endif

    delete m_display;
}

void Compositor::frameFinished(Surface *surface)
{
    if (surface && m_dirty_surfaces.contains(surface)) {
        m_dirty_surfaces.remove(surface);
        surface->sendFrameCallback();
    } else if (!surface) {
        QSet<Surface *> dirty = m_dirty_surfaces;
        m_dirty_surfaces.clear();
        foreach (Surface *surface, dirty)
            surface->sendFrameCallback();
    }
}

void Compositor::createSurface(struct wl_client *client, uint32_t id)
{
    Surface *surface = new Surface(client,id, this);

    m_surfaces << surface;

    m_qt_compositor->surfaceCreated(surface->waylandSurface());
}

struct wl_client *Compositor::getClientFromWinId(uint winId) const
{
    Surface *surface = getSurfaceFromWinId(winId);
    if (surface)
        return surface->base()->resource.client;

    return 0;
}

Surface *Compositor::getSurfaceFromWinId(uint winId) const
{
    foreach (Surface *surface, m_surfaces) {
        if (surface->id() == winId)
            return surface;
    }

    return 0;
}

QImage Compositor::image(uint winId) const
{
    foreach (Surface *surface, m_surfaces) {
        if (surface->id() == winId) {
            return surface->image();
        }
    }

    return QImage();
}

uint Compositor::currentTimeMsecs()
{
    //### we throw away the time information
    struct timeval tv;
    int ret = gettimeofday(&tv, 0);
    if (ret == 0)
        return tv.tv_sec*1000 + tv.tv_usec/1000;
    return 0;
}

void Compositor::releaseBuffer(SurfaceBuffer *screenBuffer)
{
    screenBuffer->scheduledRelease();
}

void Compositor::processWaylandEvents()
{
    int ret = wl_event_loop_dispatch(m_loop, 0);
    if (ret)
        fprintf(stderr, "wl_event_loop_dispatch error: %d\n", ret);
}

void Compositor::surfaceDestroyed(Surface *surface)
{
    if (defaultInputDevice()->mouseFocus() == surface)
        defaultInputDevice()->setMouseFocus(0, QPoint(), QPoint());
    m_surfaces.removeOne(surface);
    m_dirty_surfaces.remove(surface);
    if (m_directRenderSurface == surface)
        setDirectRenderSurface(0);
    waylandCompositor()->surfaceAboutToBeDestroyed(surface->waylandSurface());
}

void Compositor::markSurfaceAsDirty(Wayland::Surface *surface)
{
    m_dirty_surfaces.insert(surface);
}

void Compositor::destroyClientForSurface(Surface *surface)
{
    wl_client *client = surface->base()->resource.client;

    if (client) {
        m_windowManagerIntegration->removeClient(client);
        wl_client_destroy(client);
    }
}

QWindow *Compositor::window() const
{
    return m_qt_compositor->window();
}

GraphicsHardwareIntegration * Compositor::graphicsHWIntegration() const
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    return m_graphics_hw_integration;
#else
    return 0;
#endif
}

void Compositor::initializeHardwareIntegration()
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (m_graphics_hw_integration)
        m_graphics_hw_integration->initializeHardware(m_display);
#endif
}

void Compositor::initializeDefaultInputDevice()
{
    m_default_wayland_input_device = new WaylandInputDevice(m_qt_compositor);
    m_default_input_device = m_default_wayland_input_device->handle();
}

void Compositor::initializeWindowManagerProtocol()
{
    m_windowManagerIntegration->initialize(m_display);
}

void Compositor::enableSubSurfaceExtension()
{
    if (!m_subSurfaceExtension) {
        m_subSurfaceExtension = new SubSurfaceExtensionGlobal(this);
    }
}

bool Compositor::setDirectRenderSurface(Surface *surface)
{
#ifdef QT_COMPOSITOR_WAYLAND_GL
    if (!m_pageFlipper) {
        m_pageFlipper = QGuiApplication::primaryScreen()->handle()->pageFlipper();
    }

    if (m_graphics_hw_integration && m_graphics_hw_integration->setDirectRenderSurface(surface ? surface->waylandSurface() : 0)) {
        m_directRenderSurface = surface;
        return true;
    }
#else
    Q_UNUSED(surface);
#endif
    return false;
}

QList<struct wl_client *> Compositor::clients() const
{
    QList<struct wl_client *> list;
    foreach (Surface *surface, m_surfaces) {
        struct wl_client *client = surface->base()->resource.client;
        if (!list.contains(client))
            list.append(client);
    }
    return list;
}

void Compositor::setScreenOrientation(Qt::ScreenOrientation orientation)
{
    m_orientation = orientation;

    QList<struct wl_client*> clientList = clients();
    for (int i = 0; i < clientList.length(); ++i) {
        struct wl_client *client = clientList.at(i);
        Output *output = m_output_global.outputForClient(client);
        Q_ASSERT(output);
        if (output->extendedOutput()){
            output->extendedOutput()->sendOutputOrientation(orientation);
        }
    }
}

Qt::ScreenOrientation Compositor::screenOrientation() const
{
    return m_orientation;
}

void Compositor::setOutputGeometry(const QRect &geometry)
{
    m_output_global.setGeometry(geometry);
}

QRect Compositor::outputGeometry() const
{
    return m_output_global.geometry();
}

void Compositor::setClientFullScreenHint(bool value)
{
    m_windowManagerIntegration->setShowIsFullScreen(value);
}

InputDevice* Compositor::defaultInputDevice()
{
    return m_default_input_device;
}

QList<Wayland::Surface *> Compositor::surfacesForClient(wl_client *client)
{
    QList<Wayland::Surface *> ret;

    for (int i=0; i < m_surfaces.count(); ++i) {
        if (m_surfaces.at(i)->base()->resource.client == client) {
            ret.append(m_surfaces.at(i));
        }
    }
    return ret;
}

void Compositor::enableTouchExtension()
{
    if (!m_touchExtension) {
        m_touchExtension = new TouchExtensionGlobal(this);
    }
}

void Compositor::configureTouchExtension(int flags)
{
    if (m_touchExtension)
        m_touchExtension->setFlags(flags);
}

void Compositor::setRetainedSelectionWatcher(RetainedSelectionFunc func, void *param)
{
    m_retainNotify = func;
    m_retainNotifyParam = param;
}

bool Compositor::wantsRetainedSelection() const
{
    return m_retainNotify != 0;
}

void Compositor::feedRetainedSelectionData(QMimeData *data)
{
    if (m_retainNotify) {
        m_retainNotify(data, m_retainNotifyParam);
    }
}

void Compositor::scheduleReleaseBuffer(SurfaceBuffer *screenBuffer)
{
    QMetaObject::invokeMethod(this,"releaseBuffer",Q_ARG(SurfaceBuffer*,screenBuffer));
}

void Compositor::overrideSelection(QMimeData *data)
{
    m_data_device_manager->overrideSelection(*data);
}

bool Compositor::isDragging() const
{
    return false;
}

void Compositor::sendDragMoveEvent(const QPoint &global, const QPoint &local,
                                            Surface *surface)
{
    Q_UNUSED(global);
    Q_UNUSED(local);
    Q_UNUSED(surface);
//    Drag::instance()->dragMove(global, local, surface);
}

void Compositor::sendDragEndEvent()
{
//    Drag::instance()->dragEnd();
}

} // namespace Wayland
