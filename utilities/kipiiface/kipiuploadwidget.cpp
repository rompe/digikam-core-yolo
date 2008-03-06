/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select an image collection 
 *               to upload new items using digiKam album folder views
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QHeaderView>
#include <QTreeWidget>
#include <QHBoxLayout>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "constants.h"
#include "ddebug.h"
#include "album.h"
#include "albumthumbnailloader.h"
#include "treefolderitem.h"
#include "kipiinterface.h"
#include "kipiimagecollection.h"
#include "kipiuploadwidget.h"
#include "kipiuploadwidget.moc"

namespace Digikam
{

KipiUploadWidget::KipiUploadWidget(KipiInterface* iface, QWidget *parent)
                : KIPI::UploadWidget(parent)
{
    m_iface = iface;

    m_albumsView = new QTreeWidget(this);
    m_albumsView->setDragEnabled(false);
    m_albumsView->setDropIndicatorShown(false);
    m_albumsView->setAcceptDrops(false);
    m_albumsView->header()->hide();

    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->addWidget(m_albumsView);
    hlay->setMargin(0);
    hlay->setSpacing(0);

    // ------------------------------------------------------------------------------------

    populateTreeView(AlbumManager::instance()->allPAlbums(), m_albumsView); 

    // ------------------------------------------------------------------------------------

    connect(m_albumsView, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(selectionChanged()));
}

KipiUploadWidget::~KipiUploadWidget() 
{
}

void KipiUploadWidget::populateTreeView(const AlbumList& aList, QTreeWidget *view)
{
    for (AlbumList::const_iterator it = aList.begin(); it != aList.end(); ++it)
    {
        Album *album        = *it;
        TreeAlbumItem *item = 0;

        if (album->isRoot())
        {
            item = new TreeAlbumItem(view, album);
            item->setExpanded(true);
        }
        else
        {
            TreeAlbumItem* pitem = (TreeAlbumItem*)(album->parent()->extraData(view));
            if (!pitem)
            {
                DWarning() << "Failed to find parent for Album " << album->title() << endl;
                continue;
            }

            item = new TreeAlbumItem(pitem, album);
        }

        if (item)
        {
            PAlbum* palbum = dynamic_cast<PAlbum*>(album);
            if (palbum)
                item->setIcon(0, AlbumThumbnailLoader::instance()->getStandardAlbumIcon(palbum));
            else
            {
                TAlbum* talbum = dynamic_cast<TAlbum*>(album);
                if (talbum)
                    item->setIcon(0, AlbumThumbnailLoader::instance()->getStandardTagIcon(talbum));
            }

            if (album == AlbumManager::instance()->currentAlbum())
            {
                item->setExpanded(true);
                view->setCurrentItem(item);
                view->scrollToItem(item);
            }
        }
    }
}

KIPI::ImageCollection KipiUploadWidget::selectedImageCollection() const
{
    QString ext = m_iface->fileExtensions();
    KIPI::ImageCollection collection; 

    TreeAlbumItem* item = dynamic_cast<TreeAlbumItem*>(m_albumsView->currentItem());
    if (item)
        collection = new KipiImageCollection(KipiImageCollection::AllItems, item->album(), ext);

    return collection;
}

}  // namespace Digikam
