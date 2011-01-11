/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Raw import tool
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rawimport.moc"

// Qt includes

#include <QString>
#include <QLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "drawdecoding.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "curveswidget.h"
#include "imagehistogram.h"
#include "rawsettingsbox.h"
#include "rawpostprocessing.h"
#include "editortooliface.h"
#include "rawpreview.h"

namespace Digikam
{

class RawImport::RawImportPriv
{
public:

    RawImportPriv() :
        settingsBox(0),
        previewWidget(0)
    {
    }

    RawSettingsBox* settingsBox;
    RawPreview*     previewWidget;
};

RawImport::RawImport(const KUrl& url, QObject* parent)
    : EditorToolThreaded(parent), d(new RawImportPriv)
{
    d->previewWidget = new RawPreview(url, 0);
    d->settingsBox   = new RawSettingsBox(url, 0);

    setToolName(i18n("Raw Import"));
    setToolIcon(SmallIcon("kdcraw"));
    setProgressMessage(i18n("Post Processing"));
    setToolView(d->previewWidget);
    setToolSettings(d->settingsBox);
    init();
}

RawImport::~RawImport()
{
    delete d;
}

void RawImport::slotInit()
{
    EditorToolThreaded::slotInit();

    // ---------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalLoadingProgress(float)),
            this, SLOT(slotLoadingProgress(float)));

    connect(d->previewWidget, SIGNAL(signalLoadingFailed()),
            this, SLOT(slotLoadingFailed()));

    connect(d->previewWidget, SIGNAL(signalDemosaicedImage()),
            this, SLOT(slotDemosaicedImage()));

    connect(d->settingsBox, SIGNAL(signalPostProcessingChanged()),
            this, SLOT(slotTimer()));

    connect(d->settingsBox, SIGNAL(signalUpdatePreview()),
            this, SLOT(slotUpdatePreview()));

    connect(d->settingsBox, SIGNAL(signalAbortPreview()),
            this, SLOT(slotAbort()));

    // ---------------------------------------------------------------

    setBusy(true);
    slotUpdatePreview();
}

void RawImport::setBusy(bool val)
{
    if (val)
    {
        d->previewWidget->setCursor(Qt::WaitCursor);
    }
    else
    {
        d->previewWidget->unsetCursor();
    }

    d->settingsBox->setBusy(val);
}

DRawDecoding RawImport::rawDecodingSettings() const
{
    return d->settingsBox->settings();
}

DImg& RawImport::postProcessedImage() const
{
    return d->previewWidget->postProcessedImage();
}

bool RawImport::demosaicingSettingsDirty()
{
    return d->settingsBox->updateBtnEnabled();
}

void RawImport::slotUpdatePreview()
{
    DRawDecoding settings = rawDecodingSettings();
    // NOTE: we will NOT use Half Size raw extraction here, because we cannot check effects of demosaicing options in this mode.
    d->previewWidget->setDecodingSettings(settings);
}

void RawImport::slotAbort()
{
    // If preview loading, don't play with threaded filter interface.
    if (renderingMode() == EditorToolThreaded::NoneRendering)
    {
        d->previewWidget->cancelLoading();
        d->settingsBox->histogramBox()->histogram()->stopHistogramComputation();
        EditorToolIface::editorToolIface()->setToolStopProgress();
        setBusy(false);
        return;
    }

    EditorToolThreaded::slotAbort();
}

void RawImport::slotLoadingStarted()
{
    d->settingsBox->enableUpdateBtn(false);
    d->settingsBox->histogramBox()->histogram()->setDataLoading();
    d->settingsBox->curvesWidget()->setDataLoading();
    EditorToolIface::editorToolIface()->setToolStartProgress(i18n("Raw Decoding"));
    setBusy(true);
}

void RawImport::slotDemosaicedImage()
{
    d->settingsBox->setDemosaicedImage(d->previewWidget->demosaicedImage());
    slotEffect();
}

void RawImport::prepareEffect()
{
    DImg postImg = d->previewWidget->demosaicedImage();
    setFilter(dynamic_cast<DImgThreadedFilter*>(new RawPostProcessing(&postImg, this, rawDecodingSettings())));
}

void RawImport::putPreviewData()
{
    d->previewWidget->setPostProcessedImage(filter()->getTargetImage());
    d->settingsBox->setPostProcessedImage(d->previewWidget->postProcessedImage());
    EditorToolIface::editorToolIface()->setToolStopProgress();
    setBusy(false);
}

void RawImport::slotLoadingFailed()
{
    d->settingsBox->histogramBox()->histogram()->setLoadingFailed();
    EditorToolIface::editorToolIface()->setToolStopProgress();
    setBusy(false);
}

void RawImport::slotLoadingProgress(float v)
{
    EditorToolIface::editorToolIface()->setToolProgress((int)(v*100));
}

void RawImport::slotScaleChanged()
{
    d->settingsBox->curvesWidget()->setScaleType(d->settingsBox->histogramBox()->scale());
}

void RawImport::slotOk()
{
    // NOTE: work around B.K.O #211810
    if (d->settingsBox->curvesWidget()->isSixteenBits() != d->settingsBox->settings().rawPrm.sixteenBitsImage)
    {
        d->settingsBox->curvesWidget()->updateData(0, 0, 0, d->settingsBox->settings().rawPrm.sixteenBitsImage);
    }

    EditorTool::slotOk();
}

void RawImport::slotCancel()
{
    EditorTool::slotCancel();
}

void RawImport::exposureSettingsChanged()
{
    d->previewWidget->exposureSettingsChanged();
}

void RawImport::ICCSettingsChanged()
{
    d->previewWidget->ICCSettingsChanged();
}

} // namespace Digikam
