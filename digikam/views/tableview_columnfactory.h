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

#ifndef TABLEVIEW_COLUMNFACTORY_H
#define TABLEVIEW_COLUMNFACTORY_H

// Qt includes

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QWidget>
// #include <QModelIndex>

// KDE includes

#include <kconfiggroup.h>

// local includes

// #include "imagefiltermodel.h"
#include "tableview_model.h"
#include "tableview_shared.h"

class QModelIndex;
class QStyleOptionViewItem;

namespace Digikam
{

class ImageInfo;
class ThumbnailSize;

class TableViewColumnConfiguration
{
public:
    explicit TableViewColumnConfiguration(const QString& id = QString())
      : columnId(id),
        columnSettings()
    {
    }

    QString columnId;
    QHash<QString, QString> columnSettings;

    QString getSetting(const QString& key, const QString& defaultValue = QString()) const
    {
        if (!columnSettings.contains(key))
        {
            return defaultValue;
        }
        return columnSettings.value(key);
    }

    void loadSettings(const KConfigGroup& configGroup);
    void saveSettings(KConfigGroup& configGroup) const;
};

class TableViewColumnDescription
{
public:
    explicit TableViewColumnDescription()
      : columnId(),
        columnTitle(),
        columnIcon(),
        columnSettings(),
        subColumns()
    {
    }

    explicit TableViewColumnDescription(const QString& id, const QString title, const QString& settingKey = QString(), const QString& settingValue = QString())
      : columnId(id),
        columnTitle(title),
        columnIcon(),
        columnSettings(),
        subColumns()
    {
        if (!settingKey.isEmpty())
        {
            addSetting(settingKey, settingValue);
        }
    }

    QString columnId;
    QString columnTitle;
    QString columnIcon;
    QHash<QString, QString> columnSettings;
    QList<TableViewColumnDescription> subColumns;

    void addSubColumn(const TableViewColumnDescription& subColumnDescription)
    {
        subColumns << subColumnDescription;
    }

    void addSetting(const QString& key, const QString& value)
    {
        columnSettings.insert(key, value);
    }

    TableViewColumnConfiguration toConfiguration() const
    {
        TableViewColumnConfiguration configuration;

        configuration.columnId = columnId;
        configuration.columnSettings = columnSettings;

        return configuration;
    }

    TableViewColumnDescription setIcon(const QString& iconName)
    {
        columnIcon = iconName;

        return *this;
    }
};

class TableViewColumnConfigurationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableViewColumnConfigurationWidget(
            TableViewShared* const sharedObject,
            const TableViewColumnConfiguration& currentConfiguration,
            QWidget* const parent = 0);
    virtual ~TableViewColumnConfigurationWidget();

    virtual TableViewColumnConfiguration getNewConfiguration() = 0;

    TableViewShared* const s;
    TableViewColumnConfiguration configuration;
};

class TableViewColumn : public QObject
{
    Q_OBJECT

protected:
    TableViewShared* const s;
    TableViewColumnConfiguration configuration;

public:

    enum ColumnFlag
    {
        ColumnNoFlags = 0,
        ColumnCustomPainting = 1,
        ColumnCustomSorting = 2,
        ColumnHasConfigurationWidget = 4
    };
    Q_DECLARE_FLAGS(ColumnFlags, ColumnFlag)

    enum ColumnCompareResult
    {
        CmpEqual = 0,
        CmpABiggerB = 1,
        CmpALessB = 2
    };

    explicit TableViewColumn(
            TableViewShared* const tableViewShared,
            const TableViewColumnConfiguration& pConfiguration,
            QObject* const parent = 0
        );
    virtual ~TableViewColumn();

    static TableViewColumnDescription getDescription();
    virtual TableViewColumnConfiguration getConfiguration() const;
    virtual void setConfiguration(const TableViewColumnConfiguration& newConfiguration);
    virtual TableViewColumnConfigurationWidget* getConfigurationWidget(QWidget* const parentWidget) const;
    virtual ColumnFlags getColumnFlags() const;
    virtual QString getTitle() const;

    virtual QVariant data(TableViewModel::Item* const item, const int role) const;
    virtual ColumnCompareResult compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const;
    virtual bool paint(QPainter* const painter, const QStyleOptionViewItem& option, TableViewModel::Item* const item) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, TableViewModel::Item* const item) const;
    virtual void updateThumbnailSize();

    template<class MyType> static ColumnCompareResult compareHelper(const MyType& A, const MyType& B)
    {
        if (A==B)
        {
            return CmpEqual;
        }
        else if (A>B)
        {
            return CmpABiggerB;
        }

        return CmpALessB;
    }

    template<typename columnClass> static typename columnClass::SubColumn getSubColumnIndex(const QString& subColumnId, const typename columnClass::SubColumn defaultSubColumn)
    {
        const int index = columnClass::getSubColumns().indexOf(subColumnId);
        if (index<0)
        {
            return defaultSubColumn;
        }

        return typename columnClass::SubColumn(index);
    }

    template<typename columnClass> static bool getSubColumnIndex(const QString& subColumnId, typename columnClass::SubColumn* const subColumn)
    {
        const int index = columnClass::getSubColumns().indexOf(subColumnId);
        if (index<0)
        {
            return false;
        }

        *subColumn = typename columnClass::SubColumn(index);
        return true;
    }

Q_SIGNALS:
    void signalDataChanged(const qlonglong imageId);
    void signalAllDataChanged();
};

class TableViewColumnFactory : public QObject
{
    Q_OBJECT

public:

    explicit TableViewColumnFactory(TableViewShared* const tableViewShared, QObject* parent = 0);

    QList<TableViewColumnDescription> getColumnDescriptionList();
    TableViewColumn* getColumn(const TableViewColumnConfiguration& columnConfiguration);

private:

    class Private;
    const QScopedPointer<Private> d;
    TableViewShared* const s;
};

class TableViewColumnProfile
{
public:

    TableViewColumnProfile();
    ~TableViewColumnProfile();

    QList<TableViewColumnConfiguration> columnConfigurationList;
    QString name;
    QByteArray headerState;

    void loadSettings(const KConfigGroup& configGroup);
    void saveSettings(KConfigGroup& configGroup);
};

} /* namespace Digikam */

Q_DECLARE_METATYPE(Digikam::TableViewColumnDescription)

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::TableViewColumn::ColumnFlags)

#endif // TABLEVIEW_COLUMNFACTORY_H


