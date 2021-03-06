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

#ifndef QWAYLANDREADBACKEGLINTEGRATION_H
#define QWAYLANDREADBACKEGLINTEGRATION_H

#include "gl_integration/qwaylandglintegration.h"

#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>
#include <QtCore/QEvent>
#include <QtGui/QCursor>

#include <X11/Xlib.h>

#include <EGL/egl.h>

class QWaylandReadbackEglIntegration : public QWaylandGLIntegration
{
public:
    QWaylandReadbackEglIntegration(QWaylandDisplay *display);
    ~QWaylandReadbackEglIntegration();

    void initialize();
    QWaylandWindow *createEglWindow(QWindow *window);
    QPlatformOpenGLContext *createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const;

    QWaylandDisplay *waylandDisplay() const;
    Display *xDisplay() const;
    Window rootWindow() const;
    int depth() const;

    EGLDisplay eglDisplay();

private:
    QWaylandDisplay *mWaylandDisplay;
    Display *mDisplay;
    int mScreen;
    Window mRootWindow;
    EGLDisplay mEglDisplay;

};

#endif // QWAYLANDREADBACKEGLINTEGRATION_H
