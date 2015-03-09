/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 *
 * Copyright (C) 2005-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_decorate.h"

// Qt Includes

#include <QApplication>
#include <QKeySequence>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "digikam_debug.h"
#include "inserttexttool.h"
#include "bordertool.h"
#include "texturetool.h"

namespace DigikamDecorateImagePlugin
{

K_PLUGIN_FACTORY( DecorateFactory, registerPlugin<ImagePlugin_Decorate>(); )

class ImagePlugin_Decorate::Private
{
public:

    Private() :
        textureAction(0),
        borderAction(0),
        insertTextAction(0)
    {
    }

    QAction* textureAction;
    QAction* borderAction;
    QAction* insertTextAction;
};

ImagePlugin_Decorate::ImagePlugin_Decorate(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, QLatin1String("ImagePlugin_Decorate")),
      d(new Private)
{
    // to load the rc file from digikam's installation path
    setComponentName(QLatin1String("digikam"), i18nc("to be displayed in shortcuts dialog", "Decoration plugins"));

    d->insertTextAction = new QAction(QIcon::fromTheme(QLatin1String("insert-text")), i18n("Insert Text..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_inserttext"), d->insertTextAction );
    actionCollection()->setDefaultShortcut(d->insertTextAction, Qt::SHIFT+Qt::CTRL+Qt::Key_T);
    connect(d->insertTextAction, SIGNAL(triggered(bool)),
            this, SLOT(slotInsertText()));

    d->borderAction = new QAction(QIcon::fromTheme(QLatin1String("bordertool")), i18n("Add Border..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_border"), d->borderAction );
    connect(d->borderAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBorder()));

    d->textureAction = new QAction(QIcon::fromTheme(QLatin1String("texture")), i18n("Apply Texture..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_texture"), d->textureAction );
    connect(d->textureAction, SIGNAL(triggered(bool)),
            this, SLOT(slotTexture()));

    setActionCategory(i18n("Decorate"));
    setXMLFile(QLatin1String("digikamimageplugin_decorate_ui.rc"));

    qCDebug(DIGIKAM_IMAGEPLUGINS_LOG) << "ImagePlugin_Decorate plugin loaded";
}

ImagePlugin_Decorate::~ImagePlugin_Decorate()
{
    delete d;
}

void ImagePlugin_Decorate::setEnabledActions(bool b)
{
    d->insertTextAction->setEnabled(b);
    d->borderAction->setEnabled(b);
    d->textureAction->setEnabled(b);
}

void ImagePlugin_Decorate::slotInsertText()
{
    loadTool(new InsertTextTool(this));
}

void ImagePlugin_Decorate::slotBorder()
{
    loadTool(new BorderTool(this));
}

void ImagePlugin_Decorate::slotTexture()
{
    loadTool(new TextureTool(this));
}

} // namespace DigikamDecorateImagePlugin

#include "imageplugin_decorate.moc"
