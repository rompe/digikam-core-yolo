/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-07-17
 * Description : Qt item view for images - category drawer
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

#include "importcategorydrawer.moc"
#include "importcategorydrawer.h" //TODO: Remove this line

// Qt includes

#include <QPainter>

// KDE includes

#include <KApplication>

// Local includes

#include "camitemsortsettings.h"
#include "importfiltermodel.h"

namespace Digikam
{

class ImportCategoryDrawer::ImportCategoryDrawerPriv
{
public:

    ImportCategoryDrawerPriv()
    {
        lowerSpacing = 0;
        view         = 0;
    }

    QFont                  font;
    QRect                  rect;
    QPixmap                pixmap;
    int                    lowerSpacing;
    ImportCategorizedView* view;
};

ImportCategoryDrawer::ImportCategoryDrawer(ImportCategorizedView* parent)
#if KDE_IS_VERSION(4,5,0)
    : KCategoryDrawerV3(0), d(new ImportCategoryDrawerPriv)
#else
    : d(new ImportCategoryDrawerPriv)
#endif
{
    d->view = parent;
}

ImportCategoryDrawer::~ImportCategoryDrawer()
{
    delete d;
}

int ImportCategoryDrawer::categoryHeight(const QModelIndex& /*index*/, const QStyleOption& /*option*/) const
{
    return d->rect.height() + d->lowerSpacing;
}

int ImportCategoryDrawer::maximumHeight() const
{
    return d->rect.height() + d->lowerSpacing;
}

void ImportCategoryDrawer::setLowerSpacing(int spacing)
{
    d->lowerSpacing = spacing;
}

void ImportCategoryDrawer::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    d->font = option.font;

    if (option.rect.width() != d->rect.width())
    {
        updateRectsAndPixmaps(option.rect.width());
    }
}

void ImportCategoryDrawer::invalidatePaintingCache()
{
    if (d->rect.isNull())
    {
        return;
    }

    updateRectsAndPixmaps(d->rect.width());
}

void ImportCategoryDrawer::drawCategory(const QModelIndex& index, int /*sortRole*/,
                                       const QStyleOption& option, QPainter* p) const
{
    if (option.rect.width() != d->rect.width())
    {
        const_cast<ImportCategoryDrawer*>(this)->updateRectsAndPixmaps(option.rect.width());
    }

    p->save();

    p->translate(option.rect.topLeft());

    CamItemSortSettings::CategorizationMode mode =
        (CamItemSortSettings::CategorizationMode)index.data(ImportFilterModel::CategorizationModeRole).toInt();

    p->drawPixmap(0, 0, d->pixmap);

    QFont fontBold(d->font);
    QFont fontNormal(d->font);
    fontBold.setBold(true);
    int fnSize = fontBold.pointSize();

    //    bool usePointSize;
    if (fnSize > 0)
    {
        fontBold.setPointSize(fnSize+2);
        //        usePointSize = true;
    }
    else
    {
        fnSize = fontBold.pixelSize();
        fontBold.setPixelSize(fnSize+2);
        //        usePointSize = false;
    }

    QString header;
    QString subLine;

    switch (mode)
    {
        case CamItemSortSettings::NoCategories:
            break;
        case CamItemSortSettings::OneCategory:
            viewHeaderText(index, &header, &subLine);
            break;
        case CamItemSortSettings::CategoryByFormat:
            textForFormat(index, &header, &subLine);
            break;
    }

    p->setPen(kapp->palette().color(QPalette::HighlightedText));
    p->setFont(fontBold);

    QRect tr;
    p->drawText(5, 5, d->rect.width(), d->rect.height(),
                Qt::AlignLeft | Qt::AlignTop,
                header, &tr);

    int y = tr.height() + 2;

    p->setFont(fontNormal);

    p->drawText(5, y, d->rect.width(), d->rect.height() - y,
                Qt::AlignLeft | Qt::AlignVCenter, subLine);

    p->restore();
}

void ImportCategoryDrawer::viewHeaderText(const QModelIndex& index, QString* header, QString* subLine) const
{
    //TODO: Implement viewing containing folder name.
}

void ImportCategoryDrawer::updateRectsAndPixmaps(int width)
{
    d->rect = QRect(0, 0, 0, 0);

    // Title --------------------------------------------------------

    QFont fn(d->font);
    int fnSize = fn.pointSize();
    bool usePointSize;

    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, width,
                               0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                               "XXX");
    d->rect.setHeight(tr.height());

    if (usePointSize)
    {
        fn.setPointSize(d->font.pointSize());
    }
    else
    {
        fn.setPixelSize(d->font.pixelSize());
    }

    fn.setBold(false);
    fm = QFontMetrics(fn);
    tr = fm.boundingRect(0, 0, width,
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         "XXX");

    d->rect.setHeight(d->rect.height() + tr.height() + 10);
    d->rect.setWidth(width);

    d->pixmap = QPixmap(d->rect.width(), d->rect.height());
    d->pixmap.fill(kapp->palette().color(QPalette::Highlight));
}

} // namespace Digikam