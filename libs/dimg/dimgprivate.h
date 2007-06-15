/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-15
 * Description : DImg private data members
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIMGPRIVATE_H
#define DIMGPRIVATE_H

// QT includes.

#include <Q3Shared>
#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QMap>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

// TODO: KDE4PORT: QShared have been removed to Qt4. Check if QSharedData can be
//                 used instead as well.
//                 More details: http://doc.trolltech.com/4.3/porting4.html#qshared

class DIGIKAM_EXPORT DImgPrivate : public Q3Shared
{
public:

    DImgPrivate()
    {
        null              = true;
        width             = 0;
        height            = 0;
        data              = 0;
        alpha             = false;
        sixteenBit        = false;
        isReadOnly        = false;
    }

    ~DImgPrivate()
    {
        delete [] data;
    }

    bool                    null;
    bool                    alpha;
    bool                    sixteenBit;    
    bool                    isReadOnly;    
    
    unsigned int            width;
    unsigned int            height;

    unsigned char          *data;
    
    QMap<int, QByteArray>   metaData;
    QMap<QString, QVariant> attributes;
    QMap<QString, QString>  embeddedText;

};

}  // NameSpace Digikam

#endif /* DIMGPRIVATE_H */
