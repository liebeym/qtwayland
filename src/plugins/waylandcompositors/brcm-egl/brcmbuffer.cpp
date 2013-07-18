/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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

#include "brcmbuffer.h"

#include <EGL/eglext.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext_brcm.h>

QT_BEGIN_NAMESPACE

BrcmBuffer::BrcmBuffer(QtWayland::Compositor *compositor, const QSize &size, EGLint *data, size_t count)
    : m_invertedY(false)
    , m_handle(count)
{
    Q_UNUSED(compositor);

    base()->height = size.height();
    base()->width = size.width();

    for (size_t i = 0; i < count; ++i)
        m_handle[i] = data[i];
}

BrcmBuffer::~BrcmBuffer()
{
    eglDestroyGlobalImageBRCM(handle());
}

struct wl_buffer_interface BrcmBuffer::buffer_interface = {
    BrcmBuffer::buffer_interface_destroy
};

void BrcmBuffer::buffer_interface_destroy(wl_client *client, wl_resource *buffer)
{
    Q_UNUSED(client);
    Q_UNUSED(buffer);
}

void BrcmBuffer::delete_resource(struct wl_resource *resource)
{
    delete reinterpret_cast<BrcmBuffer *>(resource);
}

QT_END_NAMESPACE
