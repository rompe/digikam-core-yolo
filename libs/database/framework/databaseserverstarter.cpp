/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-08
 * Description : database server starter
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#include "databaseserverstarter.h"

// Qt includes

#include <QString>
#include <QList>
#include <QStringList>
#include <QtGlobal>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusVariant>
#include <QVariant>
#include <QDBusReply>
#include <QProcess>
#include <QSystemSemaphore>
#include <QApplication>
#include <QThread>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

// For whatever reason, these methods are "static protected"
class sotoSleep : public QThread
{

public:

    static void sleep(unsigned long secs)
    {
        QThread::sleep(secs);
    }
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
    static void usleep(unsigned long usecs)
    {
        QThread::usleep(usecs);
    }
};

DatabaseServerStarter::DatabaseServerStarter(QObject* const parent)
    : QObject(parent)
{
}

bool DatabaseServerStarter::init()
{
    if (qDBusRegisterMetaType<DatabaseServerError>() < 0)
    {
        qCDebug(DIGIKAM_GENERAL_LOG)<<"Error while registering DatabaseServerError class.";
        return false;
    }

    return true;
}

bool DatabaseServerStarter::__init=DatabaseServerStarter::init();

DatabaseServerError DatabaseServerStarter::startServerManagerProcess(const QString& dbType)
{
    DatabaseServerError result;
    /*
     * TODO:
     * 1. Acquire semaphore lock on "DigikamDBSrvAccess"
     * 2. Check if there is an database server manager service already registered on DBus
     * 3. If not, start the database server manager
     * 4. Release semaphore lock
     */
    QSystemSemaphore sem(QLatin1String("DigikamDBSrvAccess"), 1, QSystemSemaphore::Open);
    sem.acquire();

    if (!isServerRegistered())
    {
        const QString dbServerMgrPath = QString::fromUtf8(LIBEXEC_INSTALL_DIR) + QLatin1String("/digikamdatabaseserver");

        if ( dbServerMgrPath.isEmpty() )
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "No path to digikamdatabaseserver set in server manager configuration!";
        }

        const QStringList arguments;

        bool result = QProcess::startDetached( dbServerMgrPath, arguments );

        if ( !result )
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Could not start database server manager !";
            qCDebug(DIGIKAM_GENERAL_LOG) << "executable:" << dbServerMgrPath;
            qCDebug(DIGIKAM_GENERAL_LOG) << "arguments:" << arguments;
        }
    }

    // wait until the server has successfully registered on DBUS
    // TODO Use another way for that! Sleep isn't good :-/
    for (int i=0; i<30; ++i)
    {
        if (!isServerRegistered())
        {
            sotoSleep sleepThread;
            sleepThread.msleep(250);
            sleepThread.wait();
        }
        else
        {
            break;
        }
    }

    QDBusInterface dbus_iface(QLatin1String("org.kde.digikam.DatabaseServer"), QLatin1String("/DatabaseServer"));
    QDBusMessage stateMsg = dbus_iface.call(QLatin1String("isRunning"));

    if (!stateMsg.arguments().at(0).toBool())
    {
        DatabaseServerError error;

        QList<QVariant> arguments;
        arguments.append(dbType);

        QDBusMessage reply = dbus_iface.callWithArgumentList(QDBus::Block, QLatin1String("startDatabaseProcess"), arguments);

        if (QDBusMessage::ErrorMessage==reply.type())
        {
            result.setErrorType(DatabaseServerError::StartError);
            result.setErrorText(i18n("<p><b>Error while calling the database server starter.</b></p>"
                                     "Details:\n %1", reply.errorMessage()));
        }
        else
        {
            arguments                = reply.arguments();
            QDBusVariant dbusVariant = qvariant_cast<QDBusVariant>(arguments.at(1));
            // retrieve the actual value stored in the D-Bus variant
            QVariant dbusArgument    = dbusVariant.variant();
            DatabaseServerError item = qdbus_cast<DatabaseServerError>(dbusArgument);
            result                   = item;
        }
    }

    sem.release();

    return result;
}

bool DatabaseServerStarter::isServerRegistered()
{
    QDBusConnectionInterface* const interface = QDBusConnection::sessionBus().interface();
    QDBusReply<QStringList> reply             = interface->registeredServiceNames();

    if (reply.isValid())
    {
        QStringList serviceNames = reply.value();
        return serviceNames.contains(QLatin1String("org.kde.digikam.DatabaseServer"));
    }

    return false;
}

void DatabaseServerStarter::cleanUp()
{
    // for now, do nothing, the server will terminate on itself
}

}  // namespace Digikam
