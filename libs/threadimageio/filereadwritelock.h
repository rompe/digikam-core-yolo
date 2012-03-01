/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-29
 * Description : Intra-process file i/o lock
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef FILEREADWRITELOCK_H
#define FILEREADWRITELOCK_H

// Qt includes

#include <QString>
#include <QTemporaryFile>

// Local includes

#include "digikam_export.h"


namespace Digikam
{

class FileReadWriteLockPriv;

class DIGIKAM_EXPORT FileReadWriteLockKey
{
public:

    FileReadWriteLockKey(const QString& filePath);
    ~FileReadWriteLockKey();

    void lockForRead();
    void lockForWrite();
    bool tryLockForRead();
    bool tryLockForRead(int timeout);
    bool tryLockForWrite();
    bool tryLockForWrite(int timeout);
    void unlock();

private:

    FileReadWriteLockPriv* d;
};

class DIGIKAM_EXPORT FileReadLocker
{
public:

    FileReadLocker(const QString& filePath);
    ~FileReadLocker();

private:

    FileReadWriteLockPriv* d;
};

class DIGIKAM_EXPORT FileWriteLocker
{
public:

    FileWriteLocker(const QString& filePath);
    ~FileWriteLocker();

private:

    FileReadWriteLockPriv* d;
};

class DIGIKAM_EXPORT SafeTemporaryFile : public QTemporaryFile
{
public:
    SafeTemporaryFile();
    SafeTemporaryFile(const QString& templ);
    bool open() { return open(QIODevice::ReadWrite); }
protected:
    virtual bool open(QIODevice::OpenMode);
};

}

#endif // FILEREADWRITELOCK_H
