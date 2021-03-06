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

#ifndef WLSHELLSURFACE_H
#define WLSHELLSURFACE_H

#include <wayland-server.h>

namespace Wayland {

class Compositor;
class Surface;

class Shell
{
public:
    Shell();

    static void bind_func(struct wl_client *client, void *data,
                          uint32_t version, uint32_t id);
private:
    static void get_shell_surface(struct wl_client *client,
                  struct wl_resource *resource,
                  uint32_t id,
                  struct wl_resource *surface);
    static const struct wl_shell_interface shell_interface;

};

class ShellSurface
{
public:
    ShellSurface(struct wl_client *client, uint32_t id, Surface *surface);

private:
    struct wl_resource *m_shellSurface;
    Surface *m_surface;

    static void move(struct wl_client *client,
                     struct wl_resource *shell_surface_resource,
                     struct wl_resource *input_device_super,
                     uint32_t time);
    static void resize(struct wl_client *client,
                       struct wl_resource *shell_surface_resource,
                       struct wl_resource *input_device,
                       uint32_t time,
                       uint32_t edges);
    static void set_toplevel(struct wl_client *client,
                             struct wl_resource *shell_surface_resource);
    static void set_transient(struct wl_client *client,
                              struct wl_resource *shell_surface_resource,
                              struct wl_resource *parent,
                              int x,
                              int y,
                              uint32_t flags);
    static void set_fullscreen(struct wl_client *client,
                               struct wl_resource *shell_surface_resource);
    static void set_popup(struct wl_client *client,
                          struct wl_resource *resource,
                          struct wl_resource *input_device,
                          uint32_t time,
                          struct wl_resource *parent,
                          int32_t x,
                          int32_t y,
                          uint32_t flags);

    static const struct wl_shell_surface_interface shell_surface_interface;

};

}

#endif // WLSHELLSURFACE_H
