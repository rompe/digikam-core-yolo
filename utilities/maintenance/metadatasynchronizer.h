/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with database
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATASYNCHRONIZER_H
#define METADATASYNCHRONIZER_H

// Local includes

#include "imageinfo.h"
#include "progressmanager.h"

namespace Digikam
{

class Album;

class MetadataSynchronizer : public ProgressItem
{
    Q_OBJECT

public:

    enum SyncDirection
    {
        WriteFromDatabaseToFile,
        ReadFromFileToDatabase
    };

public:

    /** Constructor which sync all pictures metadata pictures from whole Albums collection */
    MetadataSynchronizer(SyncDirection direction);

    /** Constructor which sync all pictures metadata from an Album */
    MetadataSynchronizer(Album* album, SyncDirection direction = WriteFromDatabaseToFile);

    /** Constructor which sync all pictures metadata from an images list */
    MetadataSynchronizer(const ImageInfoList& list, SyncDirection = WriteFromDatabaseToFile);

    ~MetadataSynchronizer();

Q_SIGNALS:

    void signalComplete();

private Q_SLOTS:

    void slotParseAlbums();
    void slotAlbumParsed(const ImageInfoList&);
    void slotOneAlbumIsComplete();
    void slotCancel();

private:

    void init();
    void parseList();
    void parsePicture();
    void processOneAlbum();

private:

    class MetadataSynchronizerPriv;
    MetadataSynchronizerPriv* const d;
};

}  // namespace Digikam

#endif /* METADATASYNCHRONIZER_H */