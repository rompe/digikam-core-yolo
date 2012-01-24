/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
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

#include "fileworkeriface.moc"

// KDE includes

#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "fileactionmngr_p.h"
#include "imageattributeswatch.h"
#include "imageinfotasksplitter.h"
#include "scancontroller.h"
#include "globals.h"
#include "jpegutils.h"
#include "dimg.h"

namespace Digikam
{

void FileActionMngrFileWorker::writeOrientationToFiles(const QList<ImageInfo>& infos, int orientation)
{
    d->setWriterAction(i18n("Revising Exif Orientation tags. Please wait..."));

    QStringList failedItems;

    foreach(const ImageInfo& info, infos)
    {
        //kDebug() << "Setting Exif Orientation tag to " << orientation;

        QString path                  = info.filePath();
        DMetadata metadata(path);
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata.setImageOrientation(o);

        if (!metadata.applyChanges())
        {
            failedItems.append(info.name());
        }
        else
        {
            emit imageDataChanged(path, true, true);
            KUrl url = KUrl::fromPath(path);
            ImageAttributesWatch::instance()->fileMetadataChanged(url);
        }

        d->writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to revise Exif orientation these files:"), failedItems);
    }

    d->finishedWriting(infos.size());
}

void FileActionMngrFileWorker::writeMetadataToFiles(const QList<ImageInfo>& infos)
{
    d->setWriterAction(i18n("Writing metadata to files. Please wait..."));
    d->startingToWrite(infos);

    MetadataHub hub;

    ScanController::instance()->suspendCollectionScan();
    foreach(const ImageInfo& info, infos)
    {

        hub.load(info);
        QString filePath = info.filePath();
        bool fileChanged = hub.write(filePath, MetadataHub::FullWrite);

        if (fileChanged)
        {
            ScanController::instance()->scanFileDirectly(filePath);
        }

        // hub emits fileMetadataChanged

        d->writtenToOne();
    }
    ScanController::instance()->resumeCollectionScan();

    d->finishedWriting(infos.size());
}

void FileActionMngrFileWorker::writeMetadata(const QList<ImageInfo>& infos, MetadataHub* hub)
{
    d->setWriterAction(i18n("Writing metadata to files. Please wait..."));
    d->startingToWrite(infos);

    MetadataSettingsContainer writeSettings = MetadataSettings::instance()->settings();

    ScanController::instance()->suspendCollectionScan();
    foreach(const ImageInfo& info, infos)
    {
        QString filePath = info.filePath();

        // apply to file metadata
        bool fileChanged = hub->write(filePath, MetadataHub::FullWrite, writeSettings);

        // trigger db scan (to update file size etc.)
        if (fileChanged)
        {
            ScanController::instance()->scanFileDirectly(filePath);
        }

        // hub emits fileMetadataChanged

        d->writtenToOne();
    }
    ScanController::instance()->resumeCollectionScan();

    d->finishedWriting(infos.size());
}

void FileActionMngrFileWorker::transform(const QList<ImageInfo>& infos, int action)
{
    d->setWriterAction(i18n("Transforming items. Please wait..."));
    d->startingToWrite(infos);

    QStringList failedItems;
    ScanController::instance()->suspendCollectionScan();

    foreach(const ImageInfo& info, infos)
    {
        kDebug() << info.name() << QThread::currentThread();
        QString path = info.filePath();
        QString format = info.format();
        KExiv2::ImageOrientation currentOrientation = (KExiv2::ImageOrientation)info.orientation();
        bool isRaw = info.format().startsWith(QLatin1String("RAW"));

        bool rotateAsJpeg     = false;
        bool rotateLossy      = false;
        bool rotateByMetadata = false;

        MetadataSettingsContainer::RotationBehaviorFlags behavior;
        behavior = MetadataSettings::instance()->settings().rotationBehavior;

        rotateByMetadata = (behavior & MetadataSettingsContainer::RotateByMetadataFlag);

        // Check if rotation by content, as desired, is feasible
        // We'll later check again if it was successful
        if (behavior & MetadataSettingsContainer::RotatingPixels)
        {
            if (format == "JPG" && isJpegImage(path))
            {
                rotateAsJpeg = true;
            }

            if (behavior & MetadataSettingsContainer::RotateByLossyRotation)
            {
                DImg::FORMAT format = DImg::fileFormat(path);
                switch (format)
                {
                    case DImg::JPEG:
                    case DImg::PNG:
                    case DImg::TIFF:
                    case DImg::JP2K:
                    case DImg::PGF:
                        rotateLossy = true;
                    default:
                        break;
                }
            }
        }

        KExiv2Iface::RotationMatrix matrix;
        matrix *= currentOrientation;
        matrix *= (KExiv2Iface::RotationMatrix::TransformationAction)action;
        KExiv2::ImageOrientation finalOrientation = matrix.exifOrientation();

        bool rotatedPixels = false;
        if (rotateAsJpeg)
        {
            JpegRotator rotator(path);
            rotator.setCurrentOrientation(currentOrientation);
            if (action == KExiv2Iface::RotationMatrix::NoTransformation)
            {
                rotatedPixels = rotator.autoExifTransform();
            }
            else
            {
                rotatedPixels = rotator.exifTransform((KExiv2Iface::RotationMatrix::TransformationAction)action);
            }
            if (!rotatedPixels)
            {
                failedItems.append(info.name());
            }
        }
        else if (rotateLossy)
        {
            // Non-JPEG image: DImg
            DImg image;

            if (!image.load(path))
            {
                failedItems.append(info.name());
            }
            else
            {
                if (action == KExiv2Iface::RotationMatrix::NoTransformation)
                {
                    image.rotateAndFlip(currentOrientation);
                }
                else
                {
                    image.transform(action);
                }

                // TODO: Atomic operation!!
                // prepare metadata, including to reset Exif tag
                image.prepareMetadataToSave(path, image.format(), true);
                if (image.save(path, image.detectedFormat()))
                {
                    rotatedPixels = true;
                }
                else
                {
                    failedItems.append(info.name());
                }
            }
        }

        if (rotatedPixels)
        {
            // reset for DB. Metadata is already edited.
            finalOrientation = KExiv2::ORIENTATION_NORMAL;
        }
        else if (rotateByMetadata)
        {
            // Setting the rotation flag on Raws with embedded JPEG is a mess
            // Can apply to the RAW data, or to the embedded JPEG, or to both.
            if (!isRaw)
            {
                DMetadata metadata(path);
                metadata.setImageOrientation(finalOrientation);
                metadata.applyChanges();
            }
        }

        kDebug() << "Settings database flag to" << finalOrientation;
        // DB rotation
        ImageInfo(info).setOrientation(finalOrientation);

        if (!failedItems.contains(info.name()))
        {
            emit imageDataChanged(path, true, true);
            ImageAttributesWatch::instance()->fileMetadataChanged(info.fileUrl());
        }

        d->writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to transform these files:"), failedItems);
    }

    ScanController::instance()->resumeCollectionScan();
    d->finishedWriting(infos.size());
}

} // namespace Digikam