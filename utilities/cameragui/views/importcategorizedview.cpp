/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Qt categorized item view for camera items
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importcategorizedview.moc"

// Qt includes

#include "QTimer"

// KDE includes

#include <kdebug.h>

// Local includes

#include "loadingcacheinterface.h"
#include "imageselectionoverlay.h"
#include "camitemsortsettings.h"
#include "itemviewtooltip.h"
#include "importdelegate.h"

namespace Digikam
{

class ImportItemViewToolTip : public ItemViewToolTip
{
public:

    ImportItemViewToolTip(ImportCategorizedView* view)
        : ItemViewToolTip(view)
    {
    }

    ImportCategorizedView* view() const
    {
        return static_cast<ImportCategorizedView*>(ItemViewToolTip::view());
    }

protected:

    virtual QString tipContents()
    {
        CamItemInfo info = ImportImageModel::retrieveCamItemInfo(currentIndex());
        return QString(); //FIXME: needs to be replaced with this line (return ToolTipFiller::CamItemInfoTipContents(info);)
    }
};

class ImportCategorizedView::ImportCategorizedViewPriv
{
public:

    ImportCategorizedViewPriv() :
        model(0),
        filterModel(0),
        delegate(0),
        showToolTip(false),
        scrollToItemId(0),
        delayedEnterTimer(0),
        currentMouseEvent(0)
    {
    }

    ImportImageModel*      model;
    ImportSortFilterModel* filterModel;

    ImportDelegate*        delegate;
    bool                   showToolTip;

    qlonglong              scrollToItemId;

    QTimer*                delayedEnterTimer;

    QMouseEvent*           currentMouseEvent;
};

ImportCategorizedView::ImportCategorizedView(QWidget* parent)
    : DCategorizedView(parent), d(new ImportCategorizedViewPriv)
{
    setToolTip(new ImportItemViewToolTip(this));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(QString)));

    d->delayedEnterTimer = new QTimer(this);
    d->delayedEnterTimer->setInterval(10);
    d->delayedEnterTimer->setSingleShot(true);

    connect(d->delayedEnterTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedEnter()));
}

ImportCategorizedView::~ImportCategorizedView()
{
    d->delegate->removeAllOverlays();
    delete d;
}

//FIXME: Needs testing
void ImportCategorizedView::installDefaultModels()
{
    ImportModel* model             = new ImportModel(this);
    ImportFilterModel* filterModel = new ImportFilterModel(this);

    filterModel->setSourceImportModel(model);

    filterModel->setSortRole(CamItemSortSettings::SortByFileName);
    filterModel->setCategorizationMode(CamItemSortSettings::CategoryByFormat);
    filterModel->sort(0); // an initial sorting is necessary

    setModels(model, filterModel);
}

void ImportCategorizedView::setModels(ImportImageModel* model, ImportSortFilterModel* filterModel)
{
    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(false);
    }

    if (d->filterModel)
    {
        disconnect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
                   this, SLOT(layoutAboutToBeChanged()));

        disconnect(d->filterModel, SIGNAL(layoutChanged()),
                   this, SLOT(layoutWasChanged()));
    }

    if (d->model)
    {
        disconnect(d->model, SIGNAL(CamItemInfosAdded(QList<CamItemInfo>)),
                   this, SLOT(slotCamItemInfosAdded()));
    }

    d->model       = model;
    d->filterModel = filterModel;

    setModel(d->filterModel);

    connect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(d->filterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutWasChanged()),
            Qt::QueuedConnection);

    connect(d->model, SIGNAL(CamItemInfosAdded(QList<CamItemInfo>)),
            this, SLOT(slotCamItemInfosAdded()));

    emit modelChanged();

    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(true);
    }
}

ImportImageModel* ImportCategorizedView::importImageModel() const
{
    return d->model;
}

ImportSortFilterModel* ImportCategorizedView::importSortFilterModel() const
{
    return d->filterModel;
}

ImportFilterModel* ImportCategorizedView::importFilterModel() const
{
    return d->filterModel->importFilterModel();
}

ImportThumbnailModel* ImportCategorizedView::importThumbnailModel() const
{
    return qobject_cast<ImportThumbnailModel*>(d->model);
}

QSortFilterProxyModel* ImportCategorizedView::filterModel() const
{
    return d->filterModel;
}

ImportDelegate* ImportCategorizedView::delegate() const
{
    return d->delegate;
}

void ImportCategorizedView::setItemDelegate(ImportDelegate* delegate)
{
    ThumbnailSize oldSize      = thumbnailSize();
    ImportDelegate* oldDelegate = d->delegate;

    if (oldDelegate)
    {
        hideIndexNotification();
        d->delegate->setAllOverlaysActive(false);
        d->delegate->setViewOnAllOverlays(0);
        // Note: Be precise, no wildcard disconnect!
        disconnect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
                   this, SLOT(showIndexNotification(QModelIndex,QString)));
        disconnect(d->delegate, SIGNAL(hideNotification()),
                   this, SLOT(hideIndexNotification()));
    }

    d->delegate = delegate;
    d->delegate->setThumbnailSize(oldSize);

    if (oldDelegate)
    {
        d->delegate->setSpacing(oldDelegate->spacing());
    }

    DCategorizedView::setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());
    updateDelegateSizes();

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
            this, SLOT(showIndexNotification(QModelIndex,QString)));

    connect(d->delegate, SIGNAL(hideNotification()),
            this, SLOT(hideIndexNotification()));
}

CamItemInfo ImportCategorizedView::currentInfo() const
{
    return d->filterModel->camItemInfo(currentIndex());
}

KUrl ImportCategorizedView::currentUrl() const
{
    return currentInfo().url();
}

QList<CamItemInfo> ImportCategorizedView::selectedCamItemInfos() const
{
    return d->filterModel->camItemInfos(selectedIndexes());
}

QList<CamItemInfo> ImportCategorizedView::selectedCamItemInfosCurrentFirst() const
{
    QList<QModelIndex> indexes = selectedIndexes();
    QModelIndex        current = currentIndex();
    QList<CamItemInfo>   infos;

    foreach(const QModelIndex& index, indexes)
    {
        CamItemInfo info = d->filterModel->camItemInfo(index);

        if (index == current)
        {
            infos.prepend(info);
        }
        else
        {
            infos.append(info);
        }
    }

    return infos;
}

QList<CamItemInfo> ImportCategorizedView::CamItemInfos() const
{
    return d->filterModel->camItemInfosSorted();
}

KUrl::List ImportCategorizedView::urls() const
{
    QList<CamItemInfo> infos = CamItemInfos();
    KUrl::List       urls;

    foreach(const CamItemInfo& info, infos)
    {
        urls << info.url();
    }

    return urls;
}

KUrl::List ImportCategorizedView::selectedUrls() const
{
    QList<CamItemInfo> infos = selectedCamItemInfos();
    KUrl::List       urls;

    foreach(const CamItemInfo& info, infos)
    {
        urls << info.url();
    }

    return urls;
}

void ImportCategorizedView::toIndex(const KUrl& url)
{
    DCategorizedView::toIndex(d->filterModel->indexForPath(url.toLocalFile()));
}

CamItemInfo ImportCategorizedView::nextInOrder(const CamItemInfo& startingPoint, int nth)
{
    QModelIndex index = d->filterModel->indexForCamItemInfo(startingPoint);

    if (!index.isValid())
    {
        return CamItemInfo();
    }

    return d->filterModel->camItemInfo(d->filterModel->index(index.row() + nth, 0, QModelIndex()));
}

QModelIndex ImportCategorizedView::nextIndexHint(const QModelIndex& anchor, const QItemSelectionRange& removed) const
{
    QModelIndex hint = DCategorizedView::nextIndexHint(anchor, removed);
    CamItemInfo info   = d->filterModel->camItemInfo(anchor);

    //kDebug() << "Having initial hint" << hint << "for" << anchor << d->model->numberOfIndexesForCamItemInfo(info);

    // Fixes a special case of multiple (face) entries for the same image.
    // If one is removed, any entry of the same image shall be preferred.
    if (d->model->numberOfIndexesForCamItemInfo(info) > 1)
    {
        // The hint is for a different info, but we may have a hint for the same info
        if (info != d->filterModel->camItemInfo(hint))
        {
            int minDiff                            = d->filterModel->rowCount();
            QList<QModelIndex> indexesForCamItemInfo = d->filterModel->mapListFromSource(d->model->indexesForCamItemInfo(info));

            foreach(const QModelIndex& index, indexesForCamItemInfo)
            {
                if (index == anchor || !index.isValid() || removed.contains(index))
                {
                    continue;
                }

                int distance = qAbs(index.row() - anchor.row());

                if (distance < minDiff)
                {
                    minDiff = distance;
                    hint = index;
                    //kDebug() << "Chose index" << hint << "at distance" << minDiff << "to" << anchor;
                }
            }
        }
    }

    return hint;
}

ThumbnailSize ImportCategorizedView::thumbnailSize() const
{
/*
    ImportThumbnailModel *thumbModel = importThumbnailModel();
    if (thumbModel)
        return thumbModel->thumbnailSize();
*/
    if (d->delegate)
    {
        return d->delegate->thumbnailSize();
    }

    return ThumbnailSize();
}

void ImportCategorizedView::setThumbnailSize(int size)
{
    setThumbnailSize(ThumbnailSize(size));
}

//TODO: Needs testing to know if it is necessary to take the highlight size into considration.
void ImportCategorizedView::setThumbnailSize(const ThumbnailSize& s)
{
    // we abuse this pair of method calls to restore scroll position
    layoutAboutToBeChanged();
    ThumbnailSize size(s.size());
    d->delegate->setThumbnailSize(size);
    layoutWasChanged();
}

void ImportCategorizedView::setCurrentWhenAvailable(qlonglong camItemId)
{
    d->scrollToItemId = camItemId;
}

void ImportCategorizedView::setCurrentUrl(const KUrl& url)
{
    if (url.isEmpty())
    {
        clearSelection();
        setCurrentIndex(QModelIndex());
        return;
    }

    QString path      = url.toLocalFile();
    QModelIndex index = d->filterModel->indexForPath(path);

    if (!index.isValid())
    {
        return;
    }

    clearSelection();
    setCurrentIndex(index);
}

void ImportCategorizedView::setCurrentInfo(const CamItemInfo& info)
{
    QModelIndex index = d->filterModel->indexForCamItemInfo(info);
    clearSelection();
    setCurrentIndex(index);
}

void ImportCategorizedView::setSelectedUrls(const KUrl::List& urlList)
{
    QItemSelection mySelection;

    for (KUrl::List::const_iterator it = urlList.constBegin(); it!=urlList.constEnd(); ++it)
    {
        const QString path = it->path();
        const QModelIndex index = d->filterModel->indexForPath(path);

        if (!index.isValid())
        {
            kWarning() << "no QModelIndex found for" << *it;
        }
        else
        {
            // TODO: is there a better way?
            mySelection.select(index, index);
        }
    }

    clearSelection();
    selectionModel()->select(mySelection, QItemSelectionModel::Select);
}

void ImportCategorizedView::setSelectedCamItemInfos(const QList<CamItemInfo>& infos)
{
    QItemSelection mySelection;

    foreach(const CamItemInfo& info, infos)
    {
        QModelIndex index = d->filterModel->indexForCamItemInfo(info);
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ImportCategorizedView::hintAt(const CamItemInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    QModelIndex index = d->filterModel->indexForCamItemInfo(info);

    if (!index.isValid())
    {
        return;
    }

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    scrollTo(index);
}

void ImportCategorizedView::addOverlay(ImageDelegateOverlay* overlay, ImportDelegate* delegate)
{
    if (!delegate)
    {
        delegate = d->delegate;
    }

    delegate->installOverlay(overlay);

    if (delegate == d->delegate)
    {
        overlay->setView(this);
        overlay->setActive(true);
    }
}

void ImportCategorizedView::removeOverlay(ImageDelegateOverlay* overlay)
{
    ImportDelegate* delegate = dynamic_cast<ImportDelegate*>(overlay->delegate());

    if (delegate)
    {
        delegate->removeOverlay(overlay);
    }

    overlay->setView(0);
}

void ImportCategorizedView::updateGeometries()
{
    DCategorizedView::updateGeometries();
    d->delayedEnterTimer->start();
}

void ImportCategorizedView::slotDelayedEnter()
{
    // re-emit entered() for index under mouse (after layout).
    QModelIndex mouseIndex = indexAt(mapFromGlobal(QCursor::pos()));

    if (mouseIndex.isValid())
    {
        emit DigikamKCategorizedView::entered(mouseIndex);
    }
}

void ImportCategorizedView::addSelectionOverlay(ImportDelegate* delegate)
{
    addOverlay(new ImageSelectionOverlay(this), delegate);
}

void ImportCategorizedView::scrollToStoredItem()
{
    if (d->scrollToItemId)
    {
        if (d->model->hasImage(d->scrollToItemId))
        {
            QModelIndex index = d->filterModel->indexForCamItemId(d->scrollToItemId);
            setCurrentIndex(index);
            scrollToRelaxed(index, QAbstractItemView::PositionAtCenter);
            d->scrollToItemId = 0;
        }
    }
}

void ImportCategorizedView::slotCamItemInfosAdded()
{
    if (d->scrollToItemId)
    {
        scrollToStoredItem();
    }
}

void ImportCategorizedView::slotFileChanged(const QString& filePath)
{
    QModelIndex index = d->filterModel->indexForPath(filePath);

    if (index.isValid())
    {
        update(index);
    }
}

void ImportCategorizedView::indexActivated(const QModelIndex& index)
{
    CamItemInfo info = d->filterModel->camItemInfo(index);

    if (!info.isNull())
    {
        activated(info);
        emit camItemInfoActivated(info);
    }
}

void ImportCategorizedView::currentChanged(const QModelIndex& index, const QModelIndex& previous)
{
    DCategorizedView::currentChanged(index, previous);

    emit currentChanged(d->filterModel->camItemInfo(index));
}

void ImportCategorizedView::selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems)
{
    DCategorizedView::selectionChanged(selectedItems, deselectedItems);

    if (!selectedItems.isEmpty())
    {
        emit selected(d->filterModel->camItemInfos(selectedItems.indexes()));
    }

    if (!deselectedItems.isEmpty())
    {
        emit deselected(d->filterModel->camItemInfos(deselectedItems.indexes()));
    }
}

void ImportCategorizedView::activated(const CamItemInfo&)
{
    // implemented in subclass
}

void ImportCategorizedView::showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)
{
    CamItemInfo info = d->filterModel->camItemInfo(index);
    showContextMenuOnInfo(event, info);
}

void ImportCategorizedView::showContextMenuOnInfo(QContextMenuEvent*, const CamItemInfo&)
{
    // implemented in subclass
}

void ImportCategorizedView::paintEvent(QPaintEvent* e)
{
    // We want the thumbnails to be loaded in order.
    ImportThumbnailModel* thumbModel = importThumbnailModel();

    if (thumbModel)
    {
        QModelIndexList indexesToThumbnail = importFilterModel()->mapListToSource(categorizedIndexesIn(viewport()->rect()));
        d->delegate->prepareThumbnails(thumbModel, indexesToThumbnail);
    }

    DCategorizedView::paintEvent(e);
}

QItemSelectionModel* ImportCategorizedView::getSelectionModel() const
{
    return selectionModel();
}

AbstractItemDragDropHandler* ImportCategorizedView::dragDropHandler() const
{
    return d->model->dragDropHandler();
}

} // namespace Digikam