/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2012-03-12
 * @brief  a command line tool to test autocrop of DImg
 *
 * @author Copyright (C) 2012-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *         Copyright (C) 2012-2013 by Sayantan Datta
 *         <a href="mailto:sayantan dot knz at gmail dot com">sayantan dot knz at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <QFileInfo>
#include <QString>
#include <QRect>
#include <QDebug>

// Libkexiv2 includes

#include <kexiv2.h>

// Local includes

#include "dimg.h"
#include "drawdecoding.h"
#include "autocrop.h"
#include "dimgthreadedfilter.h"

using namespace Digikam;
using namespace KExiv2Iface;
using namespace KDcrawIface;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "testautocrop - test auto-crop transform";
        qDebug() << "Usage: <image>";
        return -1;
    }

    KExiv2::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));
    QString   outFilePath(input.baseName() + QLatin1String(".cropped.png"));

    RawDecodingSettings settings;
    settings.halfSizeColorImage    = false;
    settings.sixteenBitsImage      = true;
    settings.RGBInterpolate4Colors = false;
    settings.RAWQuality            = RawDecodingSettings::BILINEAR;

    DImg img(input.filePath(), 0, DRawDecoding(settings));

    AutoCrop ac(&img);
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();

    qDebug() << "Cropped image area: " << rect;

    img.crop(rect);
    img.save(outFilePath, QLatin1String("PNG"));

    KExiv2::cleanupExiv2();

    return 0;
}
