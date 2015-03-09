/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-03
 * Description : setup Image Editor tab.
 *
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

#include "setupeditor.h"

// Qt includes

#include <QCheckBox>
#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>

// KDE includes

#include <kcolorbutton.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Libkdcraw includes

#include <KDCRAW/RNumInput>
#include <KDCRAW/RWidgetUtils>

// Local includes

#include "dimg.h"
#include "histogramwidget.h"
#include "exposurecontainer.h"
#include "fullscreensettings.h"
#include "dxmlguiwindow.h"

using namespace KDcrawIface;

namespace Digikam
{

class SetupEditor::Private
{
public:

    Private() :
        themebackgroundColor(0),
        expoIndicatorMode(0),
        expoPreview(0),
        colorBox(0),
        backgroundColor(0),
        underExposureColor(0),
        overExposureColor(0),
        expoPreviewHisto(0),
        fullScreenSettings(0),
        underExposurePcents(0),
        overExposurePcents(0)
    {}

    static const QString  configGroupName;
    static const QString  configUseThemeBackgroundColorEntry;
    static const QString  configBackgroundColorEntry;
    static const QString  configUnderExposureColorEntry;
    static const QString  configOverExposureColorEntry;
    static const QString  configUnderExposurePercentsEntry;
    static const QString  configOverExposurePercentsEntry;
    static const QString  configExpoIndicatorModeEntry;

    QCheckBox*          themebackgroundColor;
    QCheckBox*          expoIndicatorMode;

    QLabel*             expoPreview;

    RHBox*              colorBox;
    KColorButton*       backgroundColor;
    KColorButton*       underExposureColor;
    KColorButton*       overExposureColor;

    HistogramWidget*    expoPreviewHisto;

    FullScreenSettings* fullScreenSettings;

    DImg                preview;

    RDoubleNumInput*    underExposurePcents;
    RDoubleNumInput*    overExposurePcents;
};

const QString SetupEditor::Private::configGroupName(QLatin1String("ImageViewer Settings"));
const QString SetupEditor::Private::configUseThemeBackgroundColorEntry(QLatin1String("UseThemeBackgroundColor"));
const QString SetupEditor::Private::configBackgroundColorEntry(QLatin1String("BackgroundColor"));
const QString SetupEditor::Private::configUnderExposureColorEntry(QLatin1String("UnderExposureColor"));
const QString SetupEditor::Private::configOverExposureColorEntry(QLatin1String("OverExposureColor"));
const QString SetupEditor::Private::configUnderExposurePercentsEntry(QLatin1String("UnderExposurePercentsEntry"));
const QString SetupEditor::Private::configOverExposurePercentsEntry(QLatin1String("OverExposurePercentsEntry"));
const QString SetupEditor::Private::configExpoIndicatorModeEntry(QLatin1String("ExpoIndicatorMode"));

// --------------------------------------------------------

SetupEditor::SetupEditor(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const interfaceOptionsGroup = new QGroupBox(i18n("Interface Options"), panel);
    QVBoxLayout* const gLayout1            = new QVBoxLayout(interfaceOptionsGroup);

    d->themebackgroundColor                = new QCheckBox(i18n("&Use theme background color"), interfaceOptionsGroup);

    d->themebackgroundColor->setWhatsThis(i18n("Enable this option to use the background theme "
                                               "color in the image editor area."));

    d->colorBox                       = new RHBox(interfaceOptionsGroup);
    QLabel*const backgroundColorlabel = new QLabel(i18n("&Background color:"), d->colorBox);
    d->backgroundColor                = new KColorButton(d->colorBox);
    backgroundColorlabel->setBuddy(d->backgroundColor);
    d->backgroundColor->setWhatsThis(i18n("Customize the background color to use "
                                          "in the image editor area."));

    gLayout1->addWidget(d->themebackgroundColor);
    gLayout1->addWidget(d->colorBox);
    gLayout1->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout1->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    d->fullScreenSettings            = new FullScreenSettings(FS_EDITOR, panel);

    // --------------------------------------------------------

    QGroupBox* const exposureOptionsGroup = new QGroupBox(i18n("Exposure Indicators"), panel);
    QVBoxLayout* const gLayout2           = new QVBoxLayout(exposureOptionsGroup);

    RHBox* const underExpoBox             = new RHBox(exposureOptionsGroup);
    QLabel* const underExpoColorlabel     = new QLabel(i18n("&Under-exposure color: "), underExpoBox);
    d->underExposureColor                 = new KColorButton(underExpoBox);
    underExpoColorlabel->setBuddy(d->underExposureColor);
    d->underExposureColor->setWhatsThis(i18n("Customize color used in image editor to identify "
                                             "under-exposed pixels."));

    RHBox* const underPcentBox        = new RHBox(exposureOptionsGroup);
    QLabel* const underExpoPcentlabel = new QLabel(i18n("Under-exposure percents: "), underPcentBox);
    d->underExposurePcents            = new RDoubleNumInput(underPcentBox);
    d->underExposurePcents->setDecimals(1);
    d->underExposurePcents->setRange(0.1, 5.0, 0.1);
    d->underExposurePcents->setDefaultValue(1.0);
    underExpoPcentlabel->setBuddy(d->underExposurePcents);
    d->underExposurePcents->setWhatsThis(i18n("Adjust the percents of the bottom of image histogram "
                                              "which will be used to check under exposed pixels."));

    RHBox* const overExpoBox         = new RHBox(exposureOptionsGroup);
    QLabel* const overExpoColorlabel = new QLabel(i18n("&Over-exposure color: "), overExpoBox);
    d->overExposureColor             = new KColorButton(overExpoBox);
    overExpoColorlabel->setBuddy(d->overExposureColor);
    d->overExposureColor->setWhatsThis(i18n("Customize color used in image editor to identify "
                                            "over-exposed pixels."));

    RHBox* const overPcentBox        = new RHBox(exposureOptionsGroup);
    QLabel* const overExpoPcentlabel = new QLabel(i18n("Over-exposure percents: "), overPcentBox);
    d->overExposurePcents            = new RDoubleNumInput(overPcentBox);
    d->overExposurePcents->setDecimals(1);
    d->overExposurePcents->setRange(0.1, 5.0, 0.1);
    d->overExposurePcents->setDefaultValue(1.0);
    overExpoPcentlabel->setBuddy(d->underExposurePcents);
    d->overExposurePcents->setWhatsThis(i18n("Adjust the percents of the top of image histogram "
                                             "which will be used to check over exposed pixels."));

    d->expoIndicatorMode = new QCheckBox(i18n("Indicate exposure as pure color"), exposureOptionsGroup);
    d->overExposureColor->setWhatsThis(i18n("If this option is enabled, over- and under-exposure indicators will be displayed "
                                            "only when pure white and pure black color matches, as all color components match "
                                            "the condition in the same time. "
                                            "Otherwise, indicators are turned on when one of the color components matches the condition."));

    QLabel* const exampleLabel = new QLabel(i18n("Example:"), exposureOptionsGroup);
    RHBox* const previewHBox   = new RHBox(exposureOptionsGroup);
    d->expoPreview             = new QLabel(previewHBox);
    QLabel* const space        = new QLabel(previewHBox);
    d->expoPreviewHisto        = new HistogramWidget(256, 128, previewHBox, false, false);
    d->preview                 = DImg(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/sample-aix.png")));

    if (!d->preview.isNull())
    {
        d->expoPreviewHisto->updateData(d->preview);
    }

    d->expoPreviewHisto->setChannelType(ColorChannels);
    d->expoPreview->setFrameStyle(QFrame::Box | QFrame::Plain);
    previewHBox->setStretchFactor(space, 10);

    gLayout2->addWidget(underExpoBox);
    gLayout2->addWidget(underPcentBox);
    gLayout2->addWidget(overExpoBox);
    gLayout2->addWidget(overPcentBox);
    gLayout2->addWidget(d->expoIndicatorMode);
    gLayout2->addWidget(exampleLabel);
    gLayout2->addWidget(previewHBox);
    gLayout2->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout2->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(d->fullScreenSettings);
    layout->addWidget(exposureOptionsGroup);
    layout->addStretch();
    layout->setMargin(0);
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->themebackgroundColor, SIGNAL(toggled(bool)),
            this, SLOT(slotThemeBackgroundColor(bool)));

    connect(d->expoIndicatorMode, SIGNAL(toggled(bool)),
            this, SLOT(slotExpoSettingsChanged()));

    connect(d->underExposureColor, SIGNAL(changed(QColor)),
            this, SLOT(slotExpoSettingsChanged()));

    connect(d->overExposureColor, SIGNAL(changed(QColor)),
            this, SLOT(slotExpoSettingsChanged()));

    connect(d->underExposurePcents, SIGNAL(valueChanged(double)),
            this, SLOT(slotExpoSettingsChanged()));

    connect(d->underExposurePcents, SIGNAL(valueChanged(double)),
            this, SLOT(slotShowUnderExpoHistogramGuide(double)));

    connect(d->overExposurePcents, SIGNAL(valueChanged(double)),
            this, SLOT(slotExpoSettingsChanged()));

    connect(d->overExposurePcents, SIGNAL(valueChanged(double)),
            this, SLOT(slotShowOverExpoHistogramGuide(double)));

    readSettings();

    // --------------------------------------------------------

    slotExpoSettingsChanged();
}

SetupEditor::~SetupEditor()
{
    delete d;
}

void SetupEditor::slotThemeBackgroundColor(bool e)
{
    d->colorBox->setEnabled(!e);
}

void SetupEditor::slotExpoSettingsChanged()
{
    ExposureSettingsContainer prm;
    prm.underExposureIndicator = true;
    prm.overExposureIndicator  = true;
    prm.exposureIndicatorMode  = d->expoIndicatorMode->isChecked();
    prm.underExposurePercent   = d->underExposurePcents->value();
    prm.overExposurePercent    = d->overExposurePcents->value();
    prm.underExposureColor     = d->underExposureColor->color();
    prm.overExposureColor      = d->overExposureColor->color();

    QPixmap pix                = d->preview.convertToPixmap();
    QPainter p(&pix);
    QImage pureColorMask       = d->preview.pureColorMask(&prm);
    QPixmap pixMask            = QPixmap::fromImage(pureColorMask);
    p.drawPixmap(0, 0, pixMask, 0, 0, pixMask.width(), pixMask.height());

    d->expoPreview->setPixmap(pix);
}

void SetupEditor::slotShowOverExpoHistogramGuide(double v)
{
    int max  = lround(255.0 - (255.0 * v / 100.0));
    DColor color(max, max, max, max, false);
    d->expoPreviewHisto->setHistogramGuideByColor(color);
}

void SetupEditor::slotShowUnderExpoHistogramGuide(double v)
{
    int min  = lround(0.0 + (255.0 * v / 100.0));
    DColor color(min, min, min, min, false);
    d->expoPreviewHisto->setHistogramGuideByColor(color);
}

void SetupEditor::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    QColor Black(Qt::black);
    QColor White(Qt::white);
    d->themebackgroundColor->setChecked(group.readEntry(d->configUseThemeBackgroundColorEntry, true));
    d->backgroundColor->setColor(group.readEntry(d->configBackgroundColorEntry,                Black));
    d->underExposureColor->setColor(group.readEntry(d->configUnderExposureColorEntry,          White));
    d->overExposureColor->setColor(group.readEntry(d->configOverExposureColorEntry,            Black));
    d->expoIndicatorMode->setChecked(group.readEntry(d->configExpoIndicatorModeEntry,          true));
    d->underExposurePcents->setValue(group.readEntry(d->configUnderExposurePercentsEntry,      1.0));
    d->overExposurePcents->setValue(group.readEntry(d->configOverExposurePercentsEntry,        1.0));
    d->fullScreenSettings->readSettings(group);
}

void SetupEditor::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configUseThemeBackgroundColorEntry, d->themebackgroundColor->isChecked());
    group.writeEntry(d->configBackgroundColorEntry,         d->backgroundColor->color());
    group.writeEntry(d->configUnderExposureColorEntry,      d->underExposureColor->color());
    group.writeEntry(d->configOverExposureColorEntry,       d->overExposureColor->color());
    group.writeEntry(d->configExpoIndicatorModeEntry,       d->expoIndicatorMode->isChecked());
    group.writeEntry(d->configUnderExposurePercentsEntry,   d->underExposurePcents->value());
    group.writeEntry(d->configOverExposurePercentsEntry,    d->overExposurePcents->value());

    d->fullScreenSettings->saveSettings(group);

    group.sync();
}

}  // namespace Digikam
