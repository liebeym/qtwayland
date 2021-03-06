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

#include "qwaylandclipboard.h"
#include "qwaylanddisplay.h"
#include "qwaylandinputdevice.h"
#include "qwaylanddataoffer.h"
#include "qwaylanddatasource.h"
#include "qwaylanddatadevicemanager.h"

QWaylandClipboard::QWaylandClipboard(QWaylandDisplay *display)
    : mDisplay(display)
{
}

QWaylandClipboard::~QWaylandClipboard()
{
}

QMimeData *QWaylandClipboard::mimeData(QClipboard::Mode mode)
{
    Q_ASSERT(mode == QClipboard::Clipboard);
    if (!mDisplay->dndSelectionHandler())
        return 0;

    QWaylandDataSource *transfer_source = mDisplay->dndSelectionHandler()->selectionTransferSource();
    if (transfer_source) { //if we have the keyboard focus and selectionTransferSource then we own the clipboard
        return transfer_source->mimeData();
    }
    return mDisplay->dndSelectionHandler()->selectionTransfer();
}

void QWaylandClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode)
{
    Q_ASSERT(mode == QClipboard::Clipboard);
    if (mDisplay->dndSelectionHandler())
        mDisplay->dndSelectionHandler()->createAndSetSelectionSource(data,mode);
}

bool QWaylandClipboard::supportsMode(QClipboard::Mode mode) const
{
    return mode == QClipboard::Clipboard;
}

