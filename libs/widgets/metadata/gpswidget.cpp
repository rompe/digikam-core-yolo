/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-22
 * Description : a tab widget to display GPS info
 *
 * Copyright 2006 by Gilles Caulier
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

/*
Any good explainations about GPS (in French) can be found at this url :
http://www.gpspassion.com/forumsen/topic.asp?TOPIC_ID=16593
*/

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// Qt includes.

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmap.h>
#include <qfile.h>

// KDE includes.

#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>

// LibExiv2 includes.

#include <exiv2/tags.hpp>
#include <exiv2/exif.hpp>

// Local includes.

#include "dmetadata.h"
#include "metadatalistview.h"
#include "worldmapwidget.h"
#include "gpswidget.h"

namespace Digikam
{
static char* ExifHumanList[] =
{
     "GPSLatitude",
     "GPSLongitude",
     "GPSAltitude",
     "-1"
};

// Standard Exif Entry list from to less important to the most important for photograph.
static char* StandardExifEntryList[] =
{
     "GPSInfo",
     "-1"
};

class GPSWidgetPriv
{

public:

    GPSWidgetPriv()
    {
        longLabel     = 0;
        latLabel      = 0;
        detailsButton = 0;
        map           = 0;
        longTitle     = 0;
        latTitle      = 0;
    }

    QLabel         *longLabel;
    QLabel         *latLabel;
    QLabel         *longTitle;
    QLabel         *latTitle;

    QStringList     tagsfilter;
    QStringList     keysFilter;
    
    QPushButton    *detailsButton;
    
    WorldMapWidget *map;
};

GPSWidget::GPSWidget(QWidget* parent, const char* name)
         : MetadataWidget(parent, name)
{
    d = new GPSWidgetPriv;
    
    for (int i=0 ; QString(StandardExifEntryList[i]) != QString("-1") ; i++)
        d->keysFilter << StandardExifEntryList[i];

    for (int i=0 ; QString(ExifHumanList[i]) != QString("-1") ; i++)
        d->tagsfilter << ExifHumanList[i];

    QWidget *gpsInfo = new QWidget(this);
    QGridLayout *layout = new QGridLayout(gpsInfo, 4, 2);

    d->map = new WorldMapWidget(gpsInfo);

    d->latTitle  = new QLabel(i18n("Latitude:"), gpsInfo);
    d->longTitle = new QLabel(i18n("Longitude:"), gpsInfo);
    d->longLabel = new QLabel(gpsInfo);
    d->latLabel  = new QLabel(gpsInfo);

    d->detailsButton = new QPushButton(i18n("More details..."), gpsInfo);

    layout->addMultiCellWidget(d->map, 0, 0, 0, 2);
    layout->addMultiCellWidget(d->latTitle, 1, 1, 0, 0);
    layout->addMultiCellWidget(d->latLabel, 1, 1, 1, 1);
    layout->addMultiCellWidget(d->longTitle, 2, 2, 0, 0);
    layout->addMultiCellWidget(d->longLabel, 2, 2, 1, 1);
    layout->addMultiCellWidget(d->detailsButton, 3, 3, 0, 0);
    layout->setColStretch(2, 10);
    layout->setRowStretch(4, 10);

    connect(d->detailsButton, SIGNAL(clicked()),
            this, SLOT(slotGPSDetails()));
            
    setUserAreaWidget(gpsInfo);
    decodeMetadata();
}

GPSWidget::~GPSWidget()
{
    delete d;
}

QString GPSWidget::getMetadataTitle(void)
{
    return i18n("Global Positionning System Informations");
}

bool GPSWidget::loadFromURL(const KURL& url)
{
    setFileName(url.path());
    
    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {    
        DMetadata metadata(url.path());
        QByteArray exifData = metadata.getExif();

        if (exifData.isEmpty())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(exifData);
    }

    return true;
}

bool GPSWidget::decodeMetadata()
{
    try
    {
        Exiv2::ExifData exifData;
        if (exifData.load((Exiv2::byte*)getMetadata().data(), getMetadata().size()) != 0)
        {
            d->latTitle->setEnabled(false);
            d->longTitle->setEnabled(false);
            d->longLabel->setEnabled(false);
            d->latLabel->setEnabled(false);
            d->map->setEnabled(false);
            d->detailsButton->setEnabled(false);
            kdDebug() << "Cannot parse EXIF metadata using Exiv2" << endl;
            return false;
        }

        exifData.sortByKey();
        
        QString     ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::ExifData::iterator md = exifData.begin(); md != exifData.end(); ++md)
        {
            QString key = QString::fromLocal8Bit(md->key().c_str());

            // Decode the tag value with a user friendly output.
            std::ostringstream os;
            os << *md;
            QString tagValue = QString::fromLocal8Bit(os.str().c_str());
            
            // We apply a filter to get only standard Exif tags, not maker notes.
            if (d->keysFilter.contains(key.section(".", 1, 1)))
                metaDataMap.insert(key, tagValue);
        }

        // Update all metadata contents.
        setMetadataMap(metaDataMap);
        bool ret = decodeGPSPosition();
        if (!ret)
        {
            d->latTitle->setEnabled(false);
            d->longTitle->setEnabled(false);
            d->longLabel->setEnabled(false);
            d->latLabel->setEnabled(false);
            d->map->setEnabled(false);
            d->detailsButton->setEnabled(false);
            setGPSPosition(0.0, 0.0);
            return false;
        }

        d->latTitle->setEnabled(true);
        d->longTitle->setEnabled(true);
        d->longLabel->setEnabled(true);
        d->latLabel->setEnabled(true);
        d->map->setEnabled(true);
        d->detailsButton->setEnabled(true);
        return true;
    }
    catch (Exiv2::Error& e)
    {
        d->latTitle->setEnabled(false);
        d->longTitle->setEnabled(false);
        d->longLabel->setEnabled(false);
        d->latLabel->setEnabled(false);
        d->map->setEnabled(false);
        d->detailsButton->setEnabled(false);
        setGPSPosition(0.0, 0.0);
        kdDebug() << "Cannot parse EXIF metadata using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

void GPSWidget::buildView(void)
{
    
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), d->keysFilter, d->tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap(), d->keysFilter, QStringList());
    }
}

QString GPSWidget::getTagTitle(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagTitle(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        kdDebug() << "Cannot get metadata tag title using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return i18n("Unknow");
    }
}

QString GPSWidget::getTagDescription(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagDesc(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {   
        kdDebug() << "Cannot get metadata tag description using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return i18n("No description available");
    }
}

void GPSWidget::setGPSPosition(double lat, double lng)
{
    QString val;
    d->latLabel->setText(QString("<b>%1</b>").arg(val.setNum(lat, 'f', 2)));
    d->longLabel->setText(QString("<b>%1</b>").arg(val.setNum(lng, 'f', 2)));
    d->map->setGPSPosition(lat, lng);
}

void GPSWidget::slotGPSDetails(void)
{
    QString val;
    QString url("http://www.mapquest.com/maps/map.adp?searchtype=address&formtype=address&latlongtype=decimal");
    url.append("&latitude=");
    url.append(val.setNum(d->map->getLatitude(), 'f', 8));
    url.append("&longitude=");
    url.append(val.setNum(d->map->getLongitude(), 'f', 8));
    
    KApplication::kApplication()->invokeBrowser(url);
}

bool GPSWidget::decodeGPSPosition(void)
{
    QString rational, num, den;
    double latitude=0.0, longitude=0.0;
    
    // Latitude decoding.
    
    QString latRef = *getMetadataMap().find("Exif.GPSInfo.GPSLatitudeRef");
    if (latRef.isEmpty()) return false;
        
    QString lat = *getMetadataMap().find("Exif.GPSInfo.GPSLatitude");
    if (lat.isEmpty()) return false;
    rational = lat.section(" ", 0, 0);
    num      = rational.section("/", 0, 0);
    den      = rational.section("/", 1, 1);
    latitude = num.toDouble()/den.toDouble();
    rational = lat.section(" ", 1, 1);
    num      = rational.section("/", 0, 0);
    den      = rational.section("/", 1, 1);
    latitude = latitude + (num.toDouble()/den.toDouble())/60.0;
    rational = lat.section(" ", 2, 2);
    num      = rational.section("/", 0, 0);
    den      = rational.section("/", 1, 1);
    latitude = latitude + (num.toDouble()/den.toDouble())/3600.0;
    
    if (latRef == "S") latitude *= -1.0;

    // Longitude decoding.
    
    QString lngRef = *getMetadataMap().find("Exif.GPSInfo.GPSLongitudeRef");
    if (lngRef.isEmpty()) return false;

    QString lng = *getMetadataMap().find("Exif.GPSInfo.GPSLongitude");
    if (lng.isEmpty()) return false;
    rational  = lng.section(" ", 0, 0);
    num       = rational.section("/", 0, 0);
    den       = rational.section("/", 1, 1);
    longitude = num.toDouble()/den.toDouble();
    rational  = lng.section(" ", 1, 1);
    num       = rational.section("/", 0, 0);
    den       = rational.section("/", 1, 1);
    longitude = longitude + (num.toDouble()/den.toDouble())/60.0;
    rational  = lng.section(" ", 2, 2);
    num       = rational.section("/", 0, 0);
    den       = rational.section("/", 1, 1);
    longitude = longitude + (num.toDouble()/den.toDouble())/3600.0;
    
    if (lngRef == "W") longitude *= -1.0;

    setGPSPosition(latitude, longitude);
    return true;
}

}  // namespace Digikam

#include "gpswidget.moc"
