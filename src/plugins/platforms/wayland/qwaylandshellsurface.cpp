/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the config.tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandshellsurface.h"

#include "qwaylanddisplay.h"
#include "qwaylandwindow.h"

QWaylandShellSurface::QWaylandShellSurface(struct wl_shell_surface *shell_surface, QWaylandWindow *window)
    : m_shell_surface(shell_surface)
    , m_window(window)
{
    wl_shell_surface_add_listener(m_shell_surface,&m_shell_surface_listener,this);
}


void QWaylandShellSurface::configure(void *data,
                                     wl_shell_surface *wl_shell_surface,
                                     uint32_t time,
                                     uint32_t edges,
                                     int32_t width,
                                     int32_t height)
{
    Q_UNUSED(wl_shell_surface);
    QWaylandShellSurface *shell_surface = static_cast<QWaylandShellSurface *>(data);
    shell_surface->m_window->configure(time,edges,0,0,width,height);
}

void QWaylandShellSurface::popup_done(void *data,
                                      struct wl_shell_surface *wl_shell_surface)
{
    Q_UNUSED(data);
    Q_UNUSED(wl_shell_surface);
}

const wl_shell_surface_listener QWaylandShellSurface::m_shell_surface_listener = {
    QWaylandShellSurface::configure,
    QWaylandShellSurface::popup_done
};
