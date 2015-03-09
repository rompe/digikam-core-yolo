/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process search on digiKam albums
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "digikamsearch.h"

// Qt includes

#include <QCoreApplication>
#include <QDBusConnection>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "digikam_export.h"
#include "haariface.h"
#include "imagelister.h"

// route progress info to KIOSlave facilities
class DuplicatesProgressObserver : public Digikam::HaarProgressObserver
{
public:

    explicit DuplicatesProgressObserver(KIO::SlaveBase* slave) : m_slave(slave)
    {
    }

    virtual void totalNumberToScan(int number)
    {
        m_slave->totalSize(number);
    }

    virtual void processedNumber(int number)
    {
        m_slave->processedSize(number);
    }

private:

    KIO::SlaveBase* m_slave;
};

// -----------------------------------------------------------------------------------------------------

kio_digikamsearch::kio_digikamsearch(const QByteArray& pool_socket, const QByteArray& app_socket)
    : SlaveBase("kio_digikamsearch", pool_socket, app_socket)
{
}

kio_digikamsearch::~kio_digikamsearch()
{
}

void kio_digikamsearch::special(const QByteArray& data)
{
    bool        duplicates = !metaData(QLatin1String("duplicates")).isEmpty();
    QUrl        url;
    int         listingType = 0;
    QDataStream ds(data);
    ds >> url;

    if (!ds.atEnd())
    {
        ds >> listingType;
    }

    qCDebug(DIGIKAM_KIOSLAVES_LOG) << "kio_digikamsearch::special " << url;

    Digikam::DatabaseUrl dbUrl(url);
    QDBusConnection::sessionBus().registerService(QString::fromUtf8("org.kde.digikam.KIO-digikamtags-%1")
                                                  .arg(QString::number(QCoreApplication::instance()->applicationPid())));
    Digikam::DatabaseAccess::setParameters((Digikam::DatabaseParameters)dbUrl);

    if (!duplicates)
    {
        int id                   = dbUrl.searchId();
        Digikam::SearchInfo info = Digikam::DatabaseAccess().db()->getSearchInfo(id);

        Digikam::ImageLister lister;
        lister.setListOnlyAvailable(metaData(QLatin1String("listOnlyAvailableImages")) == QLatin1String("true"));

        if (listingType == 0)
        {
            // send data every 200 images to be more responsive
            Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);

            if (info.type == Digikam::DatabaseSearch::HaarSearch)
            {
                lister.listHaarSearch(&receiver, info.query);
            }
            else
            {
                lister.listSearch(&receiver, info.query);
            }

            if (!receiver.hasError)
            {
                receiver.sendData();
            }
        }
        else
        {
            Digikam::ImageListerSlaveBaseReceiver receiver(this);
            // fast mode: limit results to 500
            lister.listSearch(&receiver, info.query, 500);

            if (!receiver.hasError)
            {
                receiver.sendData();
            }
        }
    }
    else
    {
        QString albumIdsString         = metaData(QLatin1String("albumids"));
        QString tagIdsString           = metaData(QLatin1String("tagids"));
        QString thresholdString        = metaData(QLatin1String("threshold"));

        // get albums to scan
        QStringList albumIdsStringList = albumIdsString.split(QLatin1Char(','));
        QStringList tagIdsStringList   = tagIdsString.split(QLatin1Char(','));
        QList<int>  albumIds;
        QList<int>  tagIds;

        {
            bool ok = true;
            int id  = 0;

            foreach(const QString& idString, albumIdsStringList)
            {
                id = idString.toInt(&ok);

                if (ok)
                {
                    albumIds << id;
                }
            }

            foreach(const QString& idString, tagIdsStringList)
            {
                id = idString.toInt(&ok);

                if (ok)
                {
                    tagIds << id;
                }
            }
        }

        if (albumIds.isEmpty() && tagIds.isEmpty())
        {
            qCDebug(DIGIKAM_KIOSLAVES_LOG) << "No album ids passed for duplicates search";
            error(KIO::ERR_INTERNAL, i18n("No album ids passed"));
            return;
        }

        // get info about threshold
        // If threshold value cannot be converted from string, we will use 0.4 instead.
        // 40% sound like the minimum value to use to have suitable results.
        bool   ok;
        double threshold = thresholdString.toDouble(&ok);

        if (!ok)
        {
            threshold = 0.4;
        }

        DuplicatesProgressObserver observer(this);

        // rebuild the duplicate albums
        Digikam::HaarIface iface;
        iface.rebuildDuplicatesAlbums(albumIds, tagIds, threshold, &observer);
    }

    finished();
}

/* KIO slave registration */

extern "C"
{
    Q_DECL_EXPORT int kdemain(int argc, char** argv)
    {
        // Needed to load SQL driver plugins
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("kio_digikamsearch"));

        qCDebug(DIGIKAM_KIOSLAVES_LOG) << "*** kio_digikamsearch started ***";

        if (argc != 4)
        {
            qCDebug(DIGIKAM_KIOSLAVES_LOG) << "Usage: kio_digikamsearch protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        kio_digikamsearch slave(argv[2], argv[3]);
        slave.dispatchLoop();

        qCDebug(DIGIKAM_KIOSLAVES_LOG) << "*** kio_digikamsearch finished ***";
        return 0;
    }
}
