/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : showFoto is a stand alone version of image
 *               editor with no support of digiKam database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes




#include <kconfig.h>
#include <kglobal.h>
#include <kimageio.h>
#include <klocalizedstring.h>

// Libkexiv2 includes

#include <kexiv2.h>
#include <libkexiv2_version.h>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

// Local includes

#include "daboutdata.h"
#include "showfoto.h"
#include "version.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
#pragma message("is setApplicationDomain necessary or does it come from CMakeLists already?")
    KLocalizedString::setApplicationDomain("digikam");

    KAboutData aboutData(QString::fromLatin1("showfoto"), // component name
                         i18n("showFoto"),                // display name
                         digiKamVersion());               // NOTE: showFoto version = digiKam version

    aboutData.setShortDescription(DAboutData::digiKamSlogan());;
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(DAboutData::copyright());
    aboutData.setOtherText(additionalInformation());
    aboutData.setHomepage(DAboutData::webProjectUrl().url());

    DAboutData::authorsRegistration(aboutData);


    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument("files", i18n("File(s) or folder(s) to open"), "[file(s) or folder(s)]");
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KExiv2Iface::KExiv2::initializeExiv2();


    QList<QUrl> urlList;
    QStringList urls = parser.positionalArguments();

    Q_FOREACH(const QString& url, urls) {
        urlList.append(QUrl(url));
    }

    parser.clearPositionalArguments();

    ShowFoto::ShowFoto* const w = new ShowFoto::ShowFoto(urlList);
#pragma message("is setTopWidget necessary?")
    //app.setTopWidget(w);
    w->show();

#pragma message("PORT QT5")
//    KLocale::global()->setMainCatalog("digikam");
//KF5 port: remove this line and define TRANSLATION_DOMAIN in CMakeLists.txt instead
//KLocale::global()->insertCatalog("libkdcraw");
//KF5 port: remove this line and define TRANSLATION_DOMAIN in CMakeLists.txt instead
//KLocale::global()->insertCatalog("libkexiv2");

    int ret = app.exec();

    KExiv2Iface::KExiv2::cleanupExiv2();

    return ret;
}
