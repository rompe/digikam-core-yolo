/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a kio-slave to process tag query on
 *               digiKam albums.
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "digikamtags.h"

// Qt includes

#include <QCoreApplication>
#include <QDBusConnection>
#include <QString>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albumdb.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "digikam_export.h"
#include "imagelister.h"
#include "imagelisterreceiver.h"

kio_digikamtagsProtocol::kio_digikamtagsProtocol(const QByteArray& pool_socket, const QByteArray& app_socket)
    : SlaveBase("kio_digikamtags", pool_socket, app_socket)
{
}

kio_digikamtagsProtocol::~kio_digikamtagsProtocol()
{
}

void kio_digikamtagsProtocol::special(const QByteArray& data)
{
    QUrl    url;
    QString filter;

    QDataStream ds(data);
    ds >> url;

    Digikam::DatabaseParameters dbParameters(url);
    QDBusConnection::sessionBus().registerService(QString::fromUtf8("org.kde.digikam.KIO-%1")
                                                  .arg(QString::number(QCoreApplication::instance()->applicationPid())));
    Digikam::DatabaseAccess::setParameters(dbParameters);

    bool folders     = (metaData(QLatin1String("folders")) == QLatin1String("true"));
    bool facefolders = (metaData(QLatin1String("facefolders")) == QLatin1String("true"));
    QString special  = metaData(QLatin1String("specialTagListing"));

    if (folders)
    {
        QMap<int, int> tagNumberMap = Digikam::DatabaseAccess().db()->getNumberOfImagesInTags();

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << tagNumberMap;
        SlaveBase::data(ba);
    }
    else if (facefolders)
    {
        QMap<QString, QMap<int, int> > facesNumberMap;
        facesNumberMap[Digikam::ImageTagPropertyName::autodetectedFace()] =
            Digikam::DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::autodetectedFace());
        facesNumberMap[Digikam::ImageTagPropertyName::tagRegion()]        =
            Digikam::DatabaseAccess().db()->getNumberOfImagesInTagProperties(Digikam::ImageTagPropertyName::tagRegion());

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << facesNumberMap;
        SlaveBase::data(ba);
    }
    else
    {
        bool recursive               = (metaData(QLatin1String("listTagsRecursively"))     == QLatin1String("true"));
        bool listOnlyAvailableImages = (metaData(QLatin1String("listOnlyAvailableImages")) == QLatin1String("true"));

        Digikam::ImageLister lister;
        lister.setRecursive(recursive);
        lister.setListOnlyAvailable(listOnlyAvailableImages);
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);

        if (!special.isNull())
        {
            QString searchXml = lister.tagSearchXml(url, special, recursive);
            lister.setAllowExtraValues(true); // pass property value as extra value, different binary protocol
            lister.listImageTagPropertySearch(&receiver, searchXml);
        }
        else
        {
            lister.list(&receiver, url);
        }

        // finish sending
        receiver.sendData();
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
        app.setApplicationName(QStringLiteral("kio_digikamtags"));

        qCDebug(DIGIKAM_KIOSLAVES_LOG) << "*** kio_digikamtag started ***";

        if (argc != 4)
        {
            qCDebug(DIGIKAM_KIOSLAVES_LOG) << "Usage: kio_digikamtags  protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        kio_digikamtagsProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        qCDebug(DIGIKAM_KIOSLAVES_LOG) << "*** kio_digikamtags finished ***";
        return 0;
    }
}
