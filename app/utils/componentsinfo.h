/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam components info dialog.
 *
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COMPONENTS_INFO_H
#define COMPONENTS_INFO_H

#include "digikam_config.h"

// Qt includes

#include <QString>
#include <QMap>
#include <QApplication>

#ifdef HAVE_KIPI

// Libkipi includes

#include <KIPI/Interface>
#include <KIPI/PluginLoader>

#endif /* HAVE_KIPI */

#ifdef HAVE_KFACE

// Libkface includes

#include <recognitiondatabase.h>

#endif /* HAVE_KFACE */

// Local includes

#include "libsinfodlg.h"
#include "rawcameradlg.h"
#include "dbstatdlg.h"
#include "applicationsettings.h"
#include "libopencv.h"

#ifdef HAVE_GPHOTO2

// LibGphoto2 includes

extern "C"
{
#include <gphoto2-version.h>
}

#endif /* HAVE_GPHOTO2 */

namespace Digikam
{

static inline void showDigikamComponentsInfo()
{
    // Set digiKam specific components info list.
    QMap<QString, QString> list;

#ifdef HAVE_GPHOTO2
    list.insert(i18n("LibGphoto2"), QLatin1String(gp_library_version(GP_VERSION_SHORT)[0]));
#else
    list.insert(i18n("LibGphoto2 support"), i18n("no"));
#endif /* HAVE_GPHOTO2 */

#ifdef HAVE_KFILEMETADATA
    list.insert(i18n("Baloo support"), i18n("Yes"));
#else
    list.insert(i18n("Baloo support"), i18n("no"));
#endif /* HAVE_KFILEMETADATA */

#ifdef HAVE_KDEPIMLIBS
    list.insert(i18n("Kdepimlibs support"), i18n("Yes"));
#else
    list.insert(i18n("Kdepimlibs support"), i18n("no"));
#endif /* HAVE_KDEPIMLIBS */

#ifdef HAVE_VIDEOPLAYER
    list.insert(i18n("QtMultimedia support"), i18n("Yes"));
#else
    list.insert(i18n("QtMultimedia support"), i18n("no"));
#endif /* HAVE_VIDEOPLAYER */

#ifdef HAVE_KIPI
    list.insert(i18n("LibKipi"),      KIPI::Interface::version());
    list.insert(i18n("Kipi-Plugins"), KIPI::PluginLoader::instance()->kipiPluginsVersion());
#else
    list.insert(i18n("LibKipi support"), i18n("no"));
#endif /* HAVE_KIPI */

#ifdef HAVE_KFACE
    list.insert(i18n("LibKface"),     KFaceIface::version());
#else
    list.insert(i18n("LibKface support"), i18n("no"));
#endif /* HAVE_KFACE */

    list.insert(i18n("LibOpenCV"),    QLatin1String(CV_VERSION));

    // Database Backend information

    QString dbBe = ApplicationSettings::instance()->getDatabaseType();
    list.insert(i18n("Database backend"), dbBe);

    if (dbBe != QLatin1String("QSQLITE"))
    {
        QString internal = ApplicationSettings::instance()->getInternalDatabaseServer() ? i18n("Yes") : i18n("No");
        list.insert(i18n("Database internal server"), internal);
    }

    LibsInfoDlg* const dlg = new LibsInfoDlg(qApp->activeWindow());
    dlg->setInfoMap(list);
    dlg->show();
}

static inline void showDigikamDatabaseStat()
{
    DBStatDlg* const dlg = new DBStatDlg(qApp->activeWindow());
    dlg->show();
}

static inline void showRawCameraList()
{
    RawCameraDlg* const dlg = new RawCameraDlg(qApp->activeWindow());
    dlg->show();
}

}  // namespace Digikam

#endif // COMPONENTS_INFO_H
