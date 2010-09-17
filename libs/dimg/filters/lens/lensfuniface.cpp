/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "lensfuniface.h"

// KDE includes

#include <kdebug.h>

namespace Digikam
{

LensFunIface::LensFunIface()
{
    m_init = false;
    init();
}

LensFunIface::~LensFunIface()
{
    if (m_init)
    {
    }
}

bool LensFunIface::init()
{
    m_lfDb       = lf_db_new();
    m_lfDb->Load();
    m_lfCameras  = m_lfDb->GetCameras();
    m_init       = true;
    m_usedLens   = NULL;

    return true;
}

void LensFunIface::setSettings(const LensFunContainer& settings)
{
    m_settings = settings;
}

bool LensFunIface::supportsDistortion()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibDistortion res;
    return m_usedLens->InterpolateDistortion(m_focalLength, res);
}

bool LensFunIface::supportsCCA()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibTCA res;
    return m_usedLens->InterpolateTCA(m_focalLength, res);
}

bool LensFunIface::supportsVig()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibVignetting res;
    return m_usedLens->InterpolateVignetting(m_focalLength, m_aperture, m_subjectDistance, res);
}

LensFunContainer::LensList LensFunIface::findLenses(const lfCamera* lfCamera, const QString& lensDesc, 
                                                const QString& lensMaker) const
{
    LensFunContainer::LensList lensList;
    const lfLens**             lfLens = 0;

    if (!lensMaker.isEmpty())
        lfLens = m_lfDb->FindLenses(lfCamera, lensMaker.toAscii().constData(), lensDesc.toAscii().constData());
    else
        lfLens = m_lfDb->FindLenses(lfCamera, NULL, lensDesc.toAscii().constData());

    while (lfLens && *lfLens)
    {
        lensList << (*lfLens);
        ++lfLens;
    }

    return lensList;
}

bool LensFunIface::findFromMetadata(const DMetadata& meta, LensFunContainer& settings) const
{
    if (meta.isEmpty())
        return false;

    PhotoInfoContainer photoInfo = meta.getPhotographInformation();
    QString make                 = photoInfo.make;
    QString model                = photoInfo.model;
    QString lens                 = photoInfo.lens;
    bool ret                     = false;

    // ------------------------------------------------------------------------------------------------

    const lfCamera** lfCamera = m_lfDb->FindCameras( make.toAscii().constData(), model.toAscii().constData() );

    if (lfCamera && *lfCamera)
    {
        settings.cameraPrt = *lfCamera;
        ret                = true;

        kDebug() << "Camera maker : " << settings.cameraPrt->Maker;
        kDebug() << "Camera model : " << settings.cameraPrt->Model;

        // ------------------------------------------------------------------------------------------------

        if (!lens.isEmpty())
        {
            // Performing lens searches.

            kDebug() << "Lens desc.   : " << lens;
            QMap<int, LensFunContainer::LensPtr> bestMatches;
            QString                              lensCutted;
            LensFunContainer::LensList           lensList;

            // In first, search in DB as well.
            lensList = findLenses(settings.cameraPrt, lens);
            if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);

            // Adapt exiv2 strings to lensfun strings for Nikon.
            lensCutted = lens;
            lensCutted.replace("Nikon ", "");
            lensCutted.replace("Zoom-", "");
            lensCutted.replace("IF-ID", "ED-IF");
            lensList = findLenses(settings.cameraPrt, lensCutted);
            kDebug() << "* Check for Nikon lens (" << lensCutted << " : " << lensList.count() << ")";
            if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);

            // Adapt exiv2 strings to lensfun strings. Some lens description use something like that :
            // "10.0 - 20.0 mm". This must be adapted like this : "10-20mm"
            lensCutted = lens;
            lensCutted.replace(QRegExp("\\.[0-9]"), "");
            lensCutted.replace(" - ", "-");
            lensCutted.replace(" mm", "mn");
            lensList = findLenses(settings.cameraPrt, lensCutted);
            kDebug() << "* Check for no maker lens (" << lensCutted << " : " << lensList.count() << ")";
            if (!lensList.isEmpty()) bestMatches.insert(lensList.count(), lensList[0]);

            // Display the results.

            if (bestMatches.isEmpty())
            {
                kDebug() << "lens matches : NOT FOUND";
                ret &= false;
            }
            else
            {
                settings.lensPtr = bestMatches[bestMatches.keys()[0]];
                kDebug() << "Lens found   : " << settings.lensPtr->Model;
            }

            // ------------------------------------------------------------------------------------------------

            settings.cropFactor = settings.lensPtr->CropFactor;
            kDebug() << "Crop Factor  : " << settings.cropFactor;

            // ------------------------------------------------------------------------------------------------

            QString temp = photoInfo.focalLength;
            if (temp.isEmpty())
            {
                kDebug() << "Focal Length : NOT FOUND";
                ret &= false;
            }
            settings.focal = temp.mid(0, temp.length() -3).toDouble(); // HACK: strip the " mm" at the end ...
            kDebug() << "Focal Length : " << settings.focal;

            // ------------------------------------------------------------------------------------------------

            temp = photoInfo.aperture;
            if (temp.isEmpty())
            {
                kDebug() << "Aperture     : NOT FOUND";
                ret &= false;
            }
            settings.aperture = temp.mid(1).toDouble();
            kDebug() << "Aperture     : " << settings.aperture;

            // ------------------------------------------------------------------------------------------------
            // Try to get subject distance value.

            // From standard Exif.
            temp = meta.getExifTagString("Exif.Photo.SubjectDistance");
            if (temp.isEmpty())
            {
                // From standard XMP.
                temp = meta.getXmpTagString("Xmp.exif.SubjectDistance");
            }
            if (temp.isEmpty())
            {
                // From Canon Makernote.
                temp = meta.getExifTagString("Exif.CanonSi.SubjectDistance");
            }
            if (temp.isEmpty())
            {
                // From Nikon Makernote.
                temp = meta.getExifTagString("Exif.NikonLd2.FocusDistance");
            }
            if (temp.isEmpty())
            {
                // From Nikon Makernote.
                temp = meta.getExifTagString("Exif.NikonLd3.FocusDistance");
            }
            // TODO: Add here others Makernotes tags.

            if (temp.isEmpty())
            {
                kDebug() << "Subject dist.: NOT FOUND";
                ret &= false;
            }

            temp     = temp.replace(" m", "");
            settings.distance = temp.toDouble();
            kDebug() << "Subject dist.: " << settings.distance;
        }
        else
        {
            ret &= false;
        }
    }

    kDebug() << "Return val.  : " << ret;

    return ret;
}

#if 0
LensFunIface::correctionData LensFunIface::getCorrectionData()
{
}
#endif

}  // namespace Digikam
