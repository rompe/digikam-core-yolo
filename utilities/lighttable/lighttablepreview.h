/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 *
 * Copyright (C) 2006-2011 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef LIGHTTABLEPREVIEW_H
#define LIGHTTABLEPREVIEW_H

// Qt includes

#include <QString>
#include <QDropEvent>
#include <QDragMoveEvent>

// Local includes

#include "imageinfo.h"
#include "imagepreviewview.h"
#include "digikam_export.h"

namespace Digikam
{

class LightTablePreview : public ImagePreviewView
{
    Q_OBJECT

public:

    LightTablePreview(QWidget* parent=0);
    ~LightTablePreview();

    void setDragAndDropEnabled(bool b);
    void setDragAndDropMessage();

Q_SIGNALS:

    void signalDroppedItems(const ImageInfoList&);

private:

    void dragMoveEvent(QDragMoveEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
};

}  // namespace Digikam

#endif /* LIGHTTABLEPREVIEW_H */
