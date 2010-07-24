/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process date query on
 *               digiKam albums.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "digikammapimages.h"

// C++ includes

#include <cstdlib>

// Qt includes

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDataStream>
#include <QFile>

// KDE includes

#include <kcomponentdata.h>
#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "digikam_export.h"
#include "imagelister.h"

kio_digikammapimages::kio_digikammapimages(const QByteArray& pool_socket,
                                   const QByteArray& app_socket)
                : SlaveBase("kio_digikammapimages", pool_socket, app_socket)
{
}

kio_digikammapimages::~kio_digikammapimages()
{
}

void kio_digikammapimages::special(const QByteArray& data)
{
/*    QString strLat1 = metaData("lat1");
    QString strLng1 = metaData("lng1");
    QString strLat2 = metaData("lat2");
    QString strLng2 = metaData("lng2");
    int lat1 = strLat1.toInt();
    int lng1 = strLng1.toInt();
    int lat2 = strLat2.toInt();
    int lng2 = strLng2.toInt();
*/
    bool wantDirectQuery = (metaData("wantDirectQuery") == "true");

    kDebug()<<"ENTERED IN SPECIAL KIO";
    
    KUrl    kurl;
    QString filter;

    QDataStream ds(data);
    ds >> kurl;

    Digikam::DatabaseUrl dbUrl(kurl);
    QDBusConnection::sessionBus().registerService(QString("org.kde.digikam.KIO-digikamtags-%1").arg(QString::number(QCoreApplication::instance()->applicationPid())));
    Digikam::DatabaseAccess::setParameters(dbUrl);

    if (wantDirectQuery)
    {
        //QMap<QDateTime, int> dateNumberMap = Digikam::DatabaseAccess().db()->getAllCreationDatesAndNumberOfImages();

        QString strLat1 = metaData("lat1");
        QString strLng1 = metaData("lng1");
        QString strLat2 = metaData("lat2");
        QString strLng2 = metaData("lng2");
        qreal lat1 = strLat1.toDouble();
        qreal lng1 = strLng1.toDouble();
        qreal lat2 = strLat2.toDouble();
        qreal lng2 = strLng2.toDouble();

        QList<QVariant> imagesInfoFromArea = Digikam::DatabaseAccess().db()->getImageIdsFromArea(lat1, lat2, lng1, lng2, 0, QString("rating"));
       // kDebug()<<"IMAGE IDS:"<<imageIds;        

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << imagesInfoFromArea;
        SlaveBase::data(ba);
    }
    else
    { 
        Digikam::ImageLister lister;
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBasePartsSendingReceiver receiver(this, 200);
        lister.list(&receiver, kurl);
        // send rest
        receiver.sendData();
    } 

    finished();
}

/* KIO slave registration */

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char **argv)
    {
        // Needed to load SQL driver plugins
        QCoreApplication app(argc, argv);

        KLocale::setMainCatalog("digikam");
        KComponentData componentData( "kio_digikammapimages" );
        KGlobal::locale();

        if (argc != 4) {
            kDebug() << "Usage: kio_digikammapimages  protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        kio_digikammapimages slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }
}
