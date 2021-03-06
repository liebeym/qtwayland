/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef QWAYLANDBUFFER_H
#define QWAYLANDBUFFER_H

#include <QtCore/QSize>
#include <QtCore/QRect>

#include <wayland-client.h>
#include <wayland-client-protocol.h>

class QWaylandBuffer {
public:
    QWaylandBuffer() { }
    virtual ~QWaylandBuffer() { }
    wl_buffer *buffer() {return mBuffer;}
    virtual QSize size() const = 0;
    inline void damage(const QRect &rect = QRect());

protected:
    struct wl_buffer *mBuffer;
};

void QWaylandBuffer::damage(const QRect &rect)
{
    if (rect.isValid())
        wl_buffer_damage(mBuffer,rect.x(),rect.y(),rect.width(),rect.height());
    else
        wl_buffer_damage(mBuffer,0,0,size().width(),size().height());
}

#endif // QWAYLANDBUFFER_H
