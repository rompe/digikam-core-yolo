/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Table view column helpers
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef TABLEVIEW_COLUMNS_H
#define TABLEVIEW_COLUMNS_H

// Qt includes

#include <QObject>
#include <QPainter>
#include <QStringList>

// KDE includes

// local includes

#include "tableview_columnfactory.h"
#include <libkgeomap/geocoordinates.h>
#include "thumbnailloadthread.h"

namespace Digikam
{

namespace TableViewColumns
{

class ColumnFileProperties : public TableViewColumn
{
    Q_OBJECT

private:

    enum SubColumn
    {
        SubColumnName = 0,
        SubColumnSize = 1
    } subColumn;

public:

    explicit ColumnFileProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        )
      : TableViewColumn(tableViewShared, pConfiguration, parent),
        subColumn(SubColumnName)
    {
        const QString& subColumnSetting = configuration.getSetting("subcolumn");
        if (subColumnSetting=="name")
        {
            subColumn = SubColumnName;
        }
        else if (subColumnSetting=="size")
        {
            subColumn = SubColumnSize;
        }
    }
    virtual ~ColumnFileProperties() { }

    static TableViewColumnDescription getDescription()
    {
        TableViewColumnDescription description(QLatin1String("file-properties"), i18n("File properties"));

        description.addSubColumn(
                TableViewColumnDescription("file-properties", i18n("Filename"), "subcolumn", "name")
            );

        description.addSubColumn(
                TableViewColumnDescription("file-properties", i18n("Size"), "subcolumn", "size")
            );

        return description;
    }

    virtual QString getTitle()
    {
        switch (subColumn)
        {
            case SubColumnName:
                return i18n("Filename");
            case SubColumnSize:
                return i18n("Size");
        }

        return QString();
    }

    virtual ColumnFlags getColumnFlags() const
    {
        if (subColumn==SubColumnSize)
        {
            return ColumnCustomSorting;
        }

        return ColumnNoFlags;
    }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        if (role!=Qt::DisplayRole)
        {
            /// @todo is this correct or does sourceIndex have column!=0?
            return sourceIndex.data(role);
        }

        const ImageInfo info = getImageInfo(sourceIndex);

        switch (subColumn)
        {
            case SubColumnName:
                return info.fileUrl().fileName();
                break;

            case SubColumnSize:
                /// @todo Needs custom sorting
                /// @todo Add configuration options for SI-prefixes
                return QString("%1").arg(info.fileSize());
                break;

        }

        return QVariant();
    }

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
    {
        const ImageInfo infoA = getImageInfo(sourceA);
        const ImageInfo infoB = getImageInfo(sourceB);

        if (subColumn==SubColumnSize)
        {
            const int sizeA = infoA.fileSize();
            const int sizeB = infoB.fileSize();

            return compareHelper<int>(sizeA, sizeB);
        }

        kWarning()<<"file: unimplemented comparison, subColumn="<<subColumn;
        return CmpEqual;
    }

};

class ColumnItemProperties : public TableViewColumn
{
    Q_OBJECT

private:

    enum SubColumn
    {
        SubColumnWidth = 0,
        SubColumnHeight = 1
    } subColumn;

public:

    explicit ColumnItemProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        )
      : TableViewColumn(tableViewShared, pConfiguration, parent),
        subColumn(SubColumnWidth)
    {
        const QString& subColumnSetting = configuration.getSetting("subcolumn");
        if (subColumnSetting=="width")
        {
            subColumn = SubColumnWidth;
        }
        else if (subColumnSetting=="height")
        {
            subColumn = SubColumnHeight;
        }
    }
    virtual ~ColumnItemProperties() { }

    static TableViewColumnDescription getDescription()
    {
        TableViewColumnDescription description(QLatin1String("item-properties"), i18n("Item properties"));

        description.addSubColumn(
                TableViewColumnDescription("item-properties", i18n("Width"), "subcolumn", "width")
            );

        description.addSubColumn(
                TableViewColumnDescription("item-properties", i18n("Height"), "subcolumn", "height")
            );

        return description;
    }

    virtual QString getTitle()
    {
        switch (subColumn)
        {
            case SubColumnWidth:
                return i18n("Width");
            case SubColumnHeight:
                return i18n("Height");
        }

        return QString();
    }

    virtual ColumnFlags getColumnFlags() const
    {
        if (   (subColumn==SubColumnHeight)
            || (subColumn==SubColumnWidth) )
        {
            return ColumnCustomSorting;
        }

        return ColumnNoFlags;
    }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        if (role!=Qt::DisplayRole)
        {
            /// @todo is this correct or does sourceIndex have column!=0?
            return sourceIndex.data(role);
        }

        const ImageInfo info = getImageInfo(sourceIndex);

        switch (subColumn)
        {
            case SubColumnWidth:
                /// @todo Needs custom sorting
                return QString("%1").arg(info.dimensions().width());
                break;

            case SubColumnHeight:
                /// @todo Needs custom sorting
                return QString("%1").arg(info.dimensions().height());
                break;

        }

        return QVariant();
    }

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
    {
        const ImageInfo infoA = getImageInfo(sourceA);
        const ImageInfo infoB = getImageInfo(sourceB);

        if (subColumn==SubColumnHeight)
        {
            const int heightA = infoA.dimensions().height();
            const int heightB = infoB.dimensions().height();

            return compareHelper<int>(heightA, heightB);
        }
        else if (subColumn==SubColumnWidth)
        {
            const int widthA = infoA.dimensions().width();
            const int widthB = infoB.dimensions().width();

            return compareHelper<int>(widthA, widthB);
        }

        kWarning()<<"item: unimplemented comparison, subColumn="<<subColumn;
        return CmpEqual;
    }
};

class ColumnGeoProperties : public TableViewColumn
{
    Q_OBJECT

public:

    enum SubColumn
    {
        SubColumnHasCoordinates = 0,
        SubColumnCoordinates = 1,
        SubColumnAltitude = 2
    } subColumn;

    explicit ColumnGeoProperties(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        )
      : TableViewColumn(tableViewShared, pConfiguration, parent),
        subColumn(SubColumnCoordinates)
    {
        const QString& subColumnSetting = configuration.getSetting("subcolumn");
        if (subColumnSetting=="hascoordinates")
        {
            subColumn = SubColumnHasCoordinates;
        }
        else if (subColumnSetting=="coordinates")
        {
            subColumn = SubColumnCoordinates;
        }
        else if (subColumnSetting=="altitude")
        {
            subColumn = SubColumnAltitude;
        }
    }
    virtual ~ColumnGeoProperties() { }
    static TableViewColumnDescription getDescription()
    {
        TableViewColumnDescription description(QLatin1String("geo-properties"), i18n("Geo properties"));

        description.addSubColumn(
                TableViewColumnDescription("geo-properties", i18n("Geotagged"), "subcolumn", "hascoordinates")
            );

        description.addSubColumn(
                TableViewColumnDescription("geo-properties", i18n("Coordinates"), "subcolumn", "coordinates")
            );

        description.addSubColumn(
                TableViewColumnDescription("geo-properties", i18n("Altitude"), "subcolumn", "altitude")
            );

        return description;
    }

    virtual QString getTitle()
    {
        switch (subColumn)
        {
            case SubColumnHasCoordinates:
                return i18n("Geotagged");
            case SubColumnCoordinates:
                return i18n("Coordinates");
            case SubColumnAltitude:
                return i18n("Altitude");
        }

        return QString();
    }

    virtual ColumnFlags getColumnFlags() const
    {
        if (subColumn==SubColumnAltitude)
        {
            return ColumnCustomSorting;
        }

        return ColumnNoFlags;
    }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        if (role!=Qt::DisplayRole)
        {
            /// @todo is this correct or does sourceIndex have column!=0?
            return sourceIndex.data(role);
        }

        const ImageInfo info = getImageInfo(sourceIndex);

        switch (subColumn)
        {
            case SubColumnHasCoordinates:
                return info.hasCoordinates() ? i18n("Yes") : i18n("No");
                break;

            case SubColumnCoordinates:
            {
                if (!info.hasCoordinates())
                {
                    return QString();
                }
                const KGeoMap::GeoCoordinates coordinates(info.latitudeNumber(), info.longitudeNumber());

                return QString("%1,%2").arg(coordinates.latString()).arg(coordinates.lonString());
                break;
            }

            case SubColumnAltitude:
            {
                /// @todo Needs custom sorting
                if ((!info.hasCoordinates())||(!info.hasAltitude()))
                {
                    return QString();
                }
                return QString("%1").arg(info.altitudeNumber());
                break;
            }
        }

        return QVariant();
    }

    virtual ColumnCompareResult compare(const QModelIndex& sourceA, const QModelIndex& sourceB) const
    {
        const ImageInfo infoA = getImageInfo(sourceA);
        const ImageInfo infoB = getImageInfo(sourceB);

        if (subColumn==SubColumnAltitude)
        {
            const bool hasAltitudeA = infoA.hasAltitude();
            const bool hasAltitudeB = infoB.hasAltitude();

            if (hasAltitudeA && hasAltitudeB)
            {
                const double altitudeA = infoA.altitudeNumber();
                const double altitudeB = infoB.altitudeNumber();

                return compareHelper<double>(altitudeA, altitudeB);
            }

            return compareHelper<int>(int(hasAltitudeA), int(hasAltitudeB));
        }

        kWarning()<<"geo: unimplemented comparison, subColumn="<<subColumn;
        return CmpEqual;
    }
};

class ColumnThumbnail : public TableViewColumn
{
    Q_OBJECT

public:

    explicit ColumnThumbnail(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        )
      : TableViewColumn(tableViewShared, pConfiguration, parent)
    {
        connect(s->thumbnailLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
                this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
    }

    virtual ~ColumnThumbnail() { }

    static TableViewColumnDescription getDescription()
    {
        return TableViewColumnDescription(QLatin1String("thumbnail"), QLatin1String("Thumbnail"));
    }

    virtual ColumnFlags getColumnFlags() const
    {
        return ColumnCustomPainting;
    }

    virtual QString getTitle() { return i18n("Thumbnail"); }

    virtual QVariant data(const QModelIndex& sourceIndex, const int role)
    {
        Q_UNUSED(sourceIndex)
        Q_UNUSED(role)

        // we do not return any data, but paint(...) something
        return QVariant();
    }

    virtual bool paint(QPainter* const painter, const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
    {
        /// @todo do we have to reset the column?
        const ImageInfo info = getImageInfo(sourceIndex);
        if (!info.isNull())
        {
            QSize size(60, 60);
            const QString path = info.filePath();
            QPixmap thumbnail;

            /// @todo handle unavailable thumbnails -> emit itemChanged(...) later
            if (s->thumbnailLoadThread->find(path, thumbnail, qMax(size.width()+2, size.height()+2)))
            {
                /// @todo Is slotThumbnailLoaded still called when the thumbnail is found right away?
                /// @todo remove borders
//                 thumbnail = thumbnail.copy(1, 1, thumbnail.size().width()-2, thumbnail.size().height()-2)
                const QSize availableSize = option.rect.size();
                const QSize pixmapSize    = thumbnail.size().boundedTo(availableSize);
                QPoint startPoint((availableSize.width()-pixmapSize.width())/2,
                                (availableSize.height()-pixmapSize.height())/2);
                startPoint+=option.rect.topLeft();
                painter->drawPixmap(QRect(startPoint, pixmapSize), thumbnail, QRect(QPoint(0, 0), pixmapSize));

                return true;
            }
        }

        // we did not get to paint a thumbnail...
        return false;
    }

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& sourceIndex) const
    {
        return QSize(60, 60);
    }

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb)
    {
        if (thumb.isNull())
        {
            return;
        }

        const QModelIndex sourceIndex = s->imageFilterModel->indexForPath(loadingDescription.filePath);
        if (!sourceIndex.isValid())
        {
            return;
        }

        emit(signalDataChanged(sourceIndex));
    }

};

} /* namespace TableViewColumns */

} /* namespace Digikam */

#endif // TABLEVIEW_COLUMNS_H

