/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : core image editor GUI implementation
 *
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "editorwindow_p.h"
#include "editorwindow.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QByteArray>
#include <QCursor>
#include <QDir>
#include <QEasingCurve>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QPointer>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QButtonGroup>
#include <QLineEdit>
#include <QKeySequence>
#include <QPushButton>
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncategory.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <kopenwithdialog.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kservicetypetrader.h>
#include <ktoolbarpopupaction.h>
#include <krun.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>

#include <kio/copyjob.h>

// Libkdcraw includes

#include <libkdcraw_version.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dmessagebox.h"
#include "applicationsettings.h"
#include "actioncategorizedview.h"
#include "buttonicondisabler.h"
#include "canvas.h"
#include "categorizeditemmodel.h"
#include "colorcorrectiondlg.h"
#include "editorcore.h"
#include "dlogoaction.h"
#include "dmetadata.h"
#include "dzoombar.h"
#include "editorstackview.h"
#include "editortool.h"
#include "editortoolsettings.h"
#include "editortooliface.h"
#include "exposurecontainer.h"
#include "fileoperation.h"
#include "filereadwritelock.h"
#include "filesaveoptionsbox.h"
#include "filesaveoptionsdlg.h"
#include "iccpostloadingmanager.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "imagedialog.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "iofilesettings.h"
#include "metadatasettings.h"
#include "libsinfodlg.h"
#include "loadingcacheinterface.h"
#include "printhelper.h"
#include "jpegsettings.h"
#include "pngsettings.h"
#include "savingcontext.h"
#include "sidebar.h"
#include "slideshowsettings.h"
#include "softproofdialog.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "versioningpromptusersavedlg.h"
#include "undostate.h"
#include "versionmanager.h"

using namespace KDcrawIface;

namespace Digikam
{

EditorWindow::EditorWindow(const QString& name)
    : DXmlGuiWindow(0),
      d(new Private)
{
    setConfigGroupName(QLatin1String("ImageViewer Settings"));
    setObjectName(name);
    setWindowFlags(Qt::Window);
    setFullScreenOptions(FS_EDITOR);

    m_nonDestructive           = true;
    m_contextMenu              = 0;
    m_servicesMenu             = 0;
    m_serviceAction            = 0;
    m_canvas                   = 0;
    m_imagePluginLoader        = 0;
    m_openVersionAction        = 0;
    m_saveAction               = 0;
    m_saveAsAction             = 0;
    m_saveCurrentVersionAction = 0;
    m_saveNewVersionAction     = 0;
    m_exportAction             = 0;
    m_revertAction             = 0;
    m_discardChangesAction     = 0;
    m_fileDeleteAction         = 0;
    m_forwardAction            = 0;
    m_backwardAction           = 0;
    m_firstAction              = 0;
    m_lastAction               = 0;
    m_applyToolAction          = 0;
    m_closeToolAction          = 0;
    m_undoAction               = 0;
    m_redoAction               = 0;
    m_selectToolsAction        = 0;
    m_showBarAction            = 0;
    m_splitter                 = 0;
    m_vSplitter                = 0;
    m_stackView                = 0;
    m_setExifOrientationTag    = true;
    m_cancelSlideShow          = false;
    m_editingOriginalImage     = true;

    // Settings containers instance.

    d->exposureSettings        = new ExposureSettingsContainer();
    d->toolIface               = new EditorToolIface(this);
    m_IOFileSettings           = new IOFileSettings();
    d->waitingLoop             = new QEventLoop(this);
}

EditorWindow::~EditorWindow()
{
    delete m_canvas;
    delete m_IOFileSettings;
    delete d->toolIface;
    delete d->exposureSettings;
    delete d;
}

EditorStackView* EditorWindow::editorStackView() const
{
    return m_stackView;
}

ExposureSettingsContainer* EditorWindow::exposureSettings() const
{
    return d->exposureSettings;
}

void EditorWindow::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    addAction2ContextMenu(QLatin1String("editorwindow_fullscreen"), true);
    addAction2ContextMenu(QLatin1String("options_show_menubar"),    true);
    m_contextMenu->addSeparator();

    // --------------------------------------------------------

    addAction2ContextMenu(QLatin1String("editorwindow_backward"), true);
    addAction2ContextMenu(QLatin1String("editorwindow_forward"),  true);
    m_contextMenu->addSeparator();

    // --------------------------------------------------------

    addAction2ContextMenu(QLatin1String("editorwindow_slideshow"),    true);
    addAction2ContextMenu(QLatin1String("editorwindow_rotate_left"),  true);
    addAction2ContextMenu(QLatin1String("editorwindow_rotate_right"), true);
    addAction2ContextMenu(QLatin1String("editorwindow_crop"),         true);
    m_contextMenu->addSeparator();

    // --------------------------------------------------------

    addAction2ContextMenu(QLatin1String("editorwindow_delete"), true);
}

void EditorWindow::setupStandardConnections()
{
    connect(m_stackView, SIGNAL(signalToggleOffFitToWindow()),
            this, SLOT(slotToggleOffFitToWindow()));

    // -- Canvas connections ------------------------------------------------

    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotForward()));

    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotBackward()));

    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(m_stackView, SIGNAL(signalZoomChanged(bool,bool,double)),
            this, SLOT(slotZoomChanged(bool,bool,double)));

    connect(m_canvas, SIGNAL(signalChanged()),
            this, SLOT(slotChanged()));

    connect(m_canvas, SIGNAL(signalAddedDropedItems(QDropEvent*)),
            this, SLOT(slotAddedDropedItems(QDropEvent*)));

    connect(m_canvas->interface(), SIGNAL(signalUndoStateChanged()),
            this, SLOT(slotUndoStateChanged()));

    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));

    connect(m_canvas, SIGNAL(signalPrepareToLoad()),
            this, SLOT(slotPrepareToLoad()));

    connect(m_canvas, SIGNAL(signalLoadingStarted(QString)),
            this, SLOT(slotLoadingStarted(QString)));

    connect(m_canvas, SIGNAL(signalLoadingFinished(QString,bool)),
            this, SLOT(slotLoadingFinished(QString,bool)));

    connect(m_canvas, SIGNAL(signalLoadingProgress(QString,float)),
            this, SLOT(slotLoadingProgress(QString,float)));

    connect(m_canvas, SIGNAL(signalSavingStarted(QString)),
            this, SLOT(slotSavingStarted(QString)));

    connect(m_canvas, SIGNAL(signalSavingFinished(QString,bool)),
            this, SLOT(slotSavingFinished(QString,bool)));

    connect(m_canvas, SIGNAL(signalSavingProgress(QString,float)),
            this, SLOT(slotSavingProgress(QString,float)));

    connect(m_canvas, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));

    connect(m_canvas, SIGNAL(signalSelectionSetText(QRect)),
            this, SLOT(slotSelectionSetText(QRect)));

    connect(m_canvas->interface(), SIGNAL(signalFileOriginChanged(QString)),
            this, SLOT(slotFileOriginChanged(QString)));

    // -- status bar connections --------------------------------------

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotNameLabelCancelButtonPressed()));

    connect(m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            d->toolIface, SLOT(slotToolAborted()));

    // -- Icc settings connections --------------------------------------

    connect(IccSettings::instance(), SIGNAL(settingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));
}

void EditorWindow::setupStandardActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    KActionCollection* const ac = actionCollection();

    m_backwardAction = buildStdAction(StdBackAction, this, SLOT(slotBackward()), this);
    ac->addAction(QLatin1String("editorwindow_backward"), m_backwardAction);
    ac->setDefaultShortcuts(m_backwardAction, QList<QKeySequence>() << Qt::Key_PageUp << Qt::Key_Backspace);

    m_forwardAction = buildStdAction(StdForwardAction, this, SLOT(slotForward()), this);
    ac->addAction(QLatin1String("editorwindow_forward"), m_forwardAction);
    ac->setDefaultShortcuts(m_forwardAction, QList<QKeySequence>() << Qt::Key_PageDown << Qt::Key_Space);

    m_firstAction = new QAction(QIcon::fromTheme(QLatin1String("go-first")), i18n("&First"), this);
    connect(m_firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    ac->addAction(QLatin1String("editorwindow_first"), m_firstAction);
    ac->setDefaultShortcuts(m_firstAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_Home);

    m_lastAction = new QAction(QIcon::fromTheme(QLatin1String("go-last")), i18n("&Last"), this);
    connect(m_lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    ac->addAction(QLatin1String("editorwindow_last"), m_lastAction);
    ac->setDefaultShortcuts(m_lastAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_End);

    m_openVersionAction = new QAction(QIcon::fromTheme(QLatin1String("image-loading")),
                                      i18nc("@action", "Open Original"), this);
    connect(m_openVersionAction, SIGNAL(triggered()), this, SLOT(slotOpenOriginal()));
    ac->addAction(QLatin1String("editorwindow_openversion"), m_openVersionAction);
    ac->setDefaultShortcuts(m_openVersionAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_End);

    m_saveAction = buildStdAction(StdSaveAction, this, SLOT(save()), this);
    ac->addAction(QLatin1String("editorwindow_save"), m_saveAction);

    m_saveAsAction = buildStdAction(StdSaveAsAction, this, SLOT(saveAs()), this);
    ac->addAction(QLatin1String("editorwindow_saveas"), m_saveAsAction);
    
    m_saveCurrentVersionAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-ok-apply")),
                                             i18nc("@action Save changes to current version", "Save Changes"), this);
    m_saveCurrentVersionAction->setToolTip(i18nc("@info:tooltip", "Save the modifications to the current version of the file"));
    connect(m_saveCurrentVersionAction, SIGNAL(triggered()), this, SLOT(saveCurrentVersion()));
    ac->addAction(QLatin1String("editorwindow_savecurrentversion"), m_saveCurrentVersionAction);

    m_saveNewVersionAction = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("list-add")),
                                                     i18nc("@action Save changes to a newly created version", "Save As New Version"), this);
    m_saveNewVersionAction->setToolTip(i18nc("@info:tooltip", "Save the current modifications to a new version of the file"));
    connect(m_saveNewVersionAction, SIGNAL(triggered()), this, SLOT(saveNewVersion()));
    ac->addAction(QLatin1String("editorwindow_savenewversion"), m_saveNewVersionAction);

#pragma "Check this action which is not plugged to action collection"
    QAction* const m_saveNewVersionAsAction = new QAction(QIcon::fromTheme(QLatin1String("document-save-as")),
                                                    i18nc("@action Save changes to a newly created version, specifying the filename and format",
                                                          "Save New Version As..."), this);
    m_saveNewVersionAsAction->setToolTip(i18nc("@info:tooltip", "Save the current modifications to a new version of the file, "
                                               "specifying the filename and format"));
    connect(m_saveNewVersionAsAction, SIGNAL(triggered()), this, SLOT(saveNewVersionAs()));
    //ac->addAction(QLatin1String("editorwindow_savenewversionas"), m_saveNewVersionAsAction);

    m_saveNewVersionInFormatAction = new QMenu(i18nc("@action Save As New Version...Save in format...",
                                                     "Save in Format"), this);
    m_saveNewVersionInFormatAction->setIcon(QIcon::fromTheme(QLatin1String("image-x-generic")));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "JPEG"),      QLatin1String("JPG"));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "TIFF"),      QLatin1String("TIFF"));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "PNG"),       QLatin1String("PNG"));
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "PGF"),       QLatin1String("PGF"));
#ifdef HAVE_JASPER
    d->plugNewVersionInFormatAction(this, m_saveNewVersionInFormatAction, i18nc("@action:inmenu", "JPEG 2000"), QLatin1String("JP2"));
#endif // HAVE_JASPER
    m_saveNewVersionAction->menu()->addAction(m_saveNewVersionAsAction);
    m_saveNewVersionAction->menu()->addAction(m_saveNewVersionInFormatAction->menuAction());

    // This also triggers saveAs, but in the context of non-destructive we want a slightly different appearance
    m_exportAction = new QAction(QIcon::fromTheme(QLatin1String("document-export")),
                                 i18nc("@action", "Export"), this);
    m_exportAction->setToolTip(i18nc("@info:tooltip", "Save the file in a folder outside your collection"));
    connect(m_exportAction, SIGNAL(triggered()), this, SLOT(saveAs()));
    ac->addAction(QLatin1String("editorwindow_export"), m_exportAction);
    ac->setDefaultShortcut(m_exportAction, Qt::CTRL + Qt::SHIFT + Qt::Key_E); // NOTE: Gimp shortcut

    m_revertAction = buildStdAction(StdRevertAction, this, SLOT(slotRevert()), this);
    ac->addAction(QLatin1String("editorwindow_revert"), m_revertAction);

    m_discardChangesAction = new QAction(QIcon::fromTheme(QLatin1String("task-reject")),
                                         i18nc("@action", "Discard Changes"), this);
    m_discardChangesAction->setToolTip(i18nc("@info:tooltip", "Discard all current changes to this file"));
    connect(m_discardChangesAction, SIGNAL(triggered()), this, SLOT(slotDiscardChanges()));
    ac->addAction(QLatin1String("editorwindow_discardchanges"), m_discardChangesAction);

    m_openVersionAction->setEnabled(false);
    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_saveCurrentVersionAction->setEnabled(false);
    m_saveNewVersionAction->setEnabled(false);
    m_revertAction->setEnabled(false);
    m_discardChangesAction->setEnabled(false);

    d->filePrintAction = new QAction(QIcon::fromTheme(QLatin1String("document-print-frame")), i18n("Print Image..."), this);
    connect(d->filePrintAction, SIGNAL(triggered()), this, SLOT(slotFilePrint()));
    ac->addAction(QLatin1String("editorwindow_print"), d->filePrintAction);
    ac->setDefaultShortcut(d->filePrintAction, Qt::CTRL + Qt::Key_P);

    QAction * const openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")), i18n("Open With Default Application"), this);
    openWithAction->setWhatsThis(i18n("Open the item with default assigned application."));
    connect(openWithAction, SIGNAL(triggered()), this, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), openWithAction);
    ac->setDefaultShortcut(openWithAction, Qt::META + Qt::Key_F4);

    m_fileDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")), i18nc("Non-pluralized", "Move to Trash"), this);
    connect(m_fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteCurrentItem()));
    ac->addAction(QLatin1String("editorwindow_delete"), m_fileDeleteAction);
    ac->setDefaultShortcut(m_fileDeleteAction, Qt::Key_Delete);

    QAction* const closeAction = buildStdAction(StdCloseAction, this, SLOT(close()), this);
    ac->addAction(QLatin1String("editorwindow_close"), closeAction);

    // -- Standard 'Edit' menu actions ---------------------------------------------

    d->copyAction = buildStdAction(StdCopyAction, m_canvas, SLOT(slotCopy()), this);
    ac->addAction(QLatin1String("editorwindow_copy"), d->copyAction);
    d->copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("edit-undo")), i18n("Undo"), this);
    m_undoAction->setEnabled(false);
    ac->addAction(QLatin1String("editorwindow_undo"), m_undoAction);
    ac->setDefaultShortcuts(m_undoAction, QList<QKeySequence>() << Qt::CTRL + Qt::Key_Z);

    connect(m_undoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));

    // we are using a signal mapper to identify which of a bunch of actions was triggered
    d->undoSignalMapper = new QSignalMapper(this);

    // connect mapper to view
    connect(d->undoSignalMapper, SIGNAL(mapped(int)),
            m_canvas, SLOT(slotUndo(int)));

    // connect simple undo action
    connect(m_undoAction, SIGNAL(triggered()), d->undoSignalMapper, SLOT(map()));
    d->undoSignalMapper->setMapping(m_undoAction, 1);

    m_redoAction = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("edit-redo")), i18n("Redo"), this);
    m_redoAction->setEnabled(false);
    ac->addAction(QLatin1String("editorwindow_redo"), m_redoAction);
    ac->setDefaultShortcuts(m_redoAction, QList<QKeySequence>() << Qt::CTRL + Qt::SHIFT + Qt::Key_Z);

    connect(m_redoAction->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));

    d->redoSignalMapper = new QSignalMapper(this);

    connect(d->redoSignalMapper, SIGNAL(mapped(int)),
            m_canvas, SLOT(slotRedo(int)));

    connect(m_redoAction, SIGNAL(triggered()), d->redoSignalMapper, SLOT(map()));
    d->redoSignalMapper->setMapping(m_redoAction, 1);

    d->selectAllAction = new QAction(i18nc("Create a selection containing the full image", "Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectAll()));
    ac->addAction(QLatin1String("editorwindow_selectAll"), d->selectAllAction);
    ac->setDefaultShortcut(d->selectAllAction, Qt::CTRL + Qt::Key_A);

    d->selectNoneAction = new QAction(i18n("Select None"), this);
    connect(d->selectNoneAction, SIGNAL(triggered()), m_canvas, SLOT(slotSelectNone()));
    ac->addAction(QLatin1String("editorwindow_selectNone"), d->selectNoneAction);
    ac->setDefaultShortcut(d->selectNoneAction, Qt::CTRL + Qt::SHIFT + Qt::Key_A);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->zoomPlusAction     = buildStdAction(StdZoomInAction, this, SLOT(slotIncreaseZoom()), this);
    QKeySequence keysPlus(d->zoomPlusAction->shortcut()[0], Qt::Key_Plus);
    ac->addAction(QLatin1String("editorwindow_zoomplus"), d->zoomPlusAction);
    ac->setDefaultShortcut(d->zoomPlusAction, keysPlus);

    d->zoomMinusAction  = buildStdAction(StdZoomOutAction, this, SLOT(slotDecreaseZoom()), this);
    QKeySequence keysMinus(d->zoomMinusAction->shortcut()[0], Qt::Key_Minus);
    ac->addAction(QLatin1String("editorwindow_zoomminus"), d->zoomMinusAction);
    ac->setDefaultShortcut(d->zoomMinusAction, keysMinus);

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")), i18n("Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), this, SLOT(slotZoomTo100Percents()));
    ac->addAction(QLatin1String("editorwindow_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, Qt::CTRL + Qt::Key_Comma);

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Fit to &Window"), this);
    d->zoomFitToWindowAction->setCheckable(true);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), this, SLOT(slotToggleFitToWindow()));
    ac->addAction(QLatin1String("editorwindow_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, Qt::ALT + Qt::CTRL + Qt::Key_E);

    d->zoomFitToSelectAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-select-fit")), i18n("Fit to &Selection"), this);
    connect(d->zoomFitToSelectAction, SIGNAL(triggered()), this, SLOT(slotFitToSelect()));
    ac->addAction(QLatin1String("editorwindow_zoomfit2select"), d->zoomFitToSelectAction);
    ac->setDefaultShortcut(d->zoomFitToSelectAction, Qt::ALT + Qt::CTRL + Qt::Key_S); // NOTE: Photoshop 7 use ALT+CTRL+0
    d->zoomFitToSelectAction->setEnabled(false);
    d->zoomFitToSelectAction->setWhatsThis(i18n("This option can be used to zoom the image to the "
                                                "current selection area."));

    // --------------------------------------------------------

    createFullScreenAction(QLatin1String("editorwindow_fullscreen"));
    createSidebarActions();

    d->slideShowAction = new QAction(QIcon::fromTheme(QLatin1String("view-presentation")), i18n("Slideshow"), this);
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotToggleSlideShow()));
    ac->addAction(QLatin1String("editorwindow_slideshow"), d->slideShowAction);
    ac->setDefaultShortcut(d->slideShowAction, Qt::Key_F9);

    d->viewUnderExpoAction = new QAction(QIcon::fromTheme(QLatin1String("underexposure")), i18n("Under-Exposure Indicator"), this);
    d->viewUnderExpoAction->setCheckable(true);
    d->viewUnderExpoAction->setWhatsThis(i18n("Set this option to display black "
                                              "overlaid on the image. This will help you to avoid "
                                              "under-exposing the image."));
    connect(d->viewUnderExpoAction, SIGNAL(triggered(bool)), this, SLOT(slotSetUnderExposureIndicator(bool)));
    ac->addAction(QLatin1String("editorwindow_underexposure"), d->viewUnderExpoAction);
    ac->setDefaultShortcut(d->viewUnderExpoAction, Qt::Key_F10);

    d->viewOverExpoAction = new QAction(QIcon::fromTheme(QLatin1String("overexposure")), i18n("Over-Exposure Indicator"), this);
    d->viewOverExpoAction->setCheckable(true);
    d->viewOverExpoAction->setWhatsThis(i18n("Set this option to display white "
                                             "overlaid on the image. This will help you to avoid "
                                             "over-exposing the image."));
    connect(d->viewOverExpoAction, SIGNAL(triggered(bool)), this, SLOT(slotSetOverExposureIndicator(bool)));
    ac->addAction(QLatin1String("editorwindow_overexposure"), d->viewOverExpoAction);
    ac->setDefaultShortcut(d->viewOverExpoAction, Qt::Key_F11);

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")), i18n("Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("editorwindow_cmview"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    d->softProofOptionsAction = new QAction(QIcon::fromTheme(QLatin1String("printer")), i18n("Soft Proofing Options..."), this);
    connect(d->softProofOptionsAction, SIGNAL(triggered()), this, SLOT(slotSoftProofingOptions()));
    ac->addAction(QLatin1String("editorwindow_softproofoptions"), d->softProofOptionsAction);

    d->viewSoftProofAction = new QAction(QIcon::fromTheme(QLatin1String("document-print-preview")), i18n("Soft Proofing View"), this);
    d->viewSoftProofAction->setCheckable(true);
    connect(d->viewSoftProofAction, SIGNAL(triggered()), this, SLOT(slotUpdateSoftProofingState()));
    ac->addAction(QLatin1String("editorwindow_softproofview"), d->viewSoftProofAction);

    // -- Standard 'Transform' menu actions ---------------------------------------------

    d->cropAction = new QAction(QIcon::fromTheme(QLatin1String("transform-crop-and-resize")), i18nc("@action", "Crop to Selection"), this);
    connect(d->cropAction, SIGNAL(triggered()), m_canvas, SLOT(slotCrop()));
    d->cropAction->setEnabled(false);
    d->cropAction->setWhatsThis(i18n("This option can be used to crop the image. "
                                     "Select a region of the image to enable this action."));
    ac->addAction(QLatin1String("editorwindow_crop"), d->cropAction);
    ac->setDefaultShortcut(d->cropAction, Qt::CTRL + Qt::Key_X);

    d->autoCropAction = new QAction(QIcon::fromTheme(QLatin1String("transform-crop")), i18nc("@action", "Auto-Crop"), this);
    d->autoCropAction->setWhatsThis(i18n("This option can be used to crop automatically the image."));
    connect(d->autoCropAction, SIGNAL(triggered()), m_canvas, SLOT(slotAutoCrop()));
    ac->addAction(QLatin1String("editorwindow_autocrop"), d->autoCropAction);
    ac->setDefaultShortcut(d->autoCropAction, Qt::SHIFT + Qt::CTRL + Qt::Key_X);

    // -- Standard 'Flip' menu actions ---------------------------------------------

    d->flipHorizAction = new QAction(QIcon::fromTheme(QLatin1String("object-flip-horizontal")), i18n("Flip Horizontally"), this);
    connect(d->flipHorizAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipHoriz()));
    ac->addAction(QLatin1String("editorwindow_flip_horiz"), d->flipHorizAction);
    ac->setDefaultShortcut(d->flipHorizAction, Qt::CTRL + Qt::Key_Asterisk);
    d->flipHorizAction->setEnabled(false);

    d->flipVertAction = new QAction(QIcon::fromTheme(QLatin1String("object-flip-vertical")), i18n("Flip Vertically"), this);
    connect(d->flipVertAction, SIGNAL(triggered()), m_canvas, SLOT(slotFlipVert()));
    ac->addAction(QLatin1String("editorwindow_flip_vert"), d->flipVertAction);
    ac->setDefaultShortcut(d->flipVertAction, Qt::CTRL + Qt::Key_Slash);
    d->flipVertAction->setEnabled(false);

    // -- Standard 'Rotate' menu actions ----------------------------------------

    d->rotateLeftAction = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-left")), i18n("Rotate Left"), this);
    connect(d->rotateLeftAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate270()));
    ac->addAction(QLatin1String("editorwindow_rotate_left"), d->rotateLeftAction);
    ac->setDefaultShortcut(d->rotateLeftAction, Qt::SHIFT + Qt::CTRL + Qt::Key_Left);
    d->rotateLeftAction->setEnabled(false);

    d->rotateRightAction = new QAction(QIcon::fromTheme(QLatin1String("object-rotate-right")), i18n("Rotate Right"), this);
    connect(d->rotateRightAction, SIGNAL(triggered()), m_canvas, SLOT(slotRotate90()));
    ac->addAction(QLatin1String("editorwindow_rotate_right"), d->rotateRightAction);
    ac->setDefaultShortcut(d->rotateRightAction, Qt::SHIFT + Qt::CTRL + Qt::Key_Right);
    d->rotateRightAction->setEnabled(false);

    // -- Standard 'Configure' menu actions ----------------------------------------

    m_showBarAction = thumbBar()->getToggleAction(this);
    ac->addAction(QLatin1String("editorwindow_showthumbs"), m_showBarAction);

    createSettingsActions();

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // ---------------------------------------------------------------------------------

    ThemeManager::instance()->registerThemeActions(this);

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -- Keyboard-only actions --------------------------------------------------------

    QAction* const altBackwardAction = new QAction(i18n("Previous Image"), this);
    ac->addAction(QLatin1String("editorwindow_backward_shift_space"), altBackwardAction);
    ac->setDefaultShortcut(altBackwardAction, Qt::SHIFT + Qt::Key_Space);
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    d->addPageUpDownActions(this, this);

    // -- Tool control actions ---------------------------------------------------------

    m_selectToolsAction = new QMenu(i18nc("@action Select image editor tool/filter", "Select Tool"), this);
    m_selectToolsAction->setIcon(QIcon::fromTheme(QLatin1String("applications-graphics")));
    m_selectToolsAction->setVisible(false);
    ac->addAction(QLatin1String("editorwindow_selecttool"), m_selectToolsAction->menuAction());

    // NOTE: setup is done after image plugins are loaded

    m_applyToolAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-ok")), i18n("Ok"), this);
    ac->addAction(QLatin1String("editorwindow_applytool"), m_applyToolAction);
    ac->setDefaultShortcut(m_applyToolAction, Qt::Key_Return);
    connect(m_applyToolAction, SIGNAL(triggered()), this, SLOT(slotApplyTool()));

    m_closeToolAction = new QAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("Cancel"), this);
    ac->addAction(QLatin1String("editorwindow_closetool"), m_closeToolAction);
    ac->setDefaultShortcut(m_closeToolAction, Qt::Key_Escape);
    connect(m_closeToolAction, SIGNAL(triggered()), this, SLOT(slotCloseTool()));

    toggleNonDestructiveActions();
    toggleToolActions();
}

void EditorWindow::setupStatusBar()
{
    m_nameLabel  = new StatusProgressBar(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_nameLabel, 100);

    d->infoLabel = new RAdjustableLabel(statusBar());
    d->infoLabel->setAdjustedText(i18n("No selection"));
    d->infoLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(d->infoLabel, 100);
    d->infoLabel->setToolTip(i18n("Information about current image selection"));

    m_resLabel   = new RAdjustableLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_resLabel, 100);
    m_resLabel->setToolTip(i18n("Information about image size"));

    d->zoomBar   = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->zoomPlusAction);
    d->zoomBar->setZoomMinusAction(d->zoomMinusAction);
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    statusBar()->addPermanentWidget(d->zoomBar);

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            m_stackView, SLOT(slotZoomSliderChanged(int)));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            m_stackView, SLOT(setZoomFactor(double)));

    d->previewToolBar = new PreviewToolBar(statusBar());
    d->previewToolBar->registerMenuActionGroup(this);
    d->previewToolBar->setEnabled(false);
    statusBar()->addPermanentWidget(d->previewToolBar);

    connect(d->previewToolBar, SIGNAL(signalPreviewModeChanged(int)),
            this, SIGNAL(signalPreviewModeChanged(int)));

    QWidget* const buttonsBox      = new QWidget(statusBar());
    QHBoxLayout* const hlay        = new QHBoxLayout(buttonsBox);
    QButtonGroup* const buttonsGrp = new QButtonGroup(buttonsBox);
    buttonsGrp->setExclusive(false);

    d->underExposureIndicator = new QToolButton(buttonsBox);
    d->underExposureIndicator->setDefaultAction(d->viewUnderExpoAction);
    d->underExposureIndicator->setFocusPolicy(Qt::NoFocus);
    //    new ButtonIconDisabler(d->underExposureIndicator);

    d->overExposureIndicator  = new QToolButton(buttonsBox);
    d->overExposureIndicator->setDefaultAction(d->viewOverExpoAction);
    d->overExposureIndicator->setFocusPolicy(Qt::NoFocus);
    //    new ButtonIconDisabler(d->overExposureIndicator);

    d->cmViewIndicator        = new QToolButton(buttonsBox);
    d->cmViewIndicator->setDefaultAction(d->viewCMViewAction);
    d->cmViewIndicator->setFocusPolicy(Qt::NoFocus);
    //    new ButtonIconDisabler(d->cmViewIndicator);

    buttonsGrp->addButton(d->underExposureIndicator);
    buttonsGrp->addButton(d->overExposureIndicator);
    buttonsGrp->addButton(d->cmViewIndicator);

    hlay->setSpacing(0);
    hlay->setMargin(0);
    hlay->addWidget(d->underExposureIndicator);
    hlay->addWidget(d->overExposureIndicator);
    hlay->addWidget(d->cmViewIndicator);

    statusBar()->addPermanentWidget(buttonsBox);
}

void EditorWindow::printImage(const QUrl&)
{
    DImg* const image = m_canvas->interface()->getImg();

    if (!image || image->isNull())
    {
        return;
    }

    PrintHelper printHelp(this);
    printHelp.print(*image);
}

void EditorWindow::slotEditKeys()
{
    editKeyboardShortcuts(d->imagepluginsActionCollection, i18nc("imageplugins shortcuts", "Image Plugins"));
}

void EditorWindow::slotAboutToShowUndoMenu()
{
    m_undoAction->menu()->clear();
    QStringList titles = m_canvas->interface()->getUndoHistory();

    for (int i = 0; i < titles.size(); ++i)
    {
        QAction* const action = m_undoAction->menu()->addAction(titles.at(i), d->undoSignalMapper, SLOT(map()));
        d->undoSignalMapper->setMapping(action, i + 1);
    }
}

void EditorWindow::slotAboutToShowRedoMenu()
{
    m_redoAction->menu()->clear();
    QStringList titles = m_canvas->interface()->getRedoHistory();

    for (int i = 0; i < titles.size(); ++i)
    {
        QAction* const action = m_redoAction->menu()->addAction(titles.at(i), d->redoSignalMapper, SLOT(map()));
        d->redoSignalMapper->setMapping(action, i + 1);
    }
}

void EditorWindow::slotIncreaseZoom()
{
    m_stackView->increaseZoom();
}

void EditorWindow::slotDecreaseZoom()
{
    m_stackView->decreaseZoom();
}

void EditorWindow::slotToggleFitToWindow()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->toggleFitToWindow();
}

void EditorWindow::slotFitToSelect()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->fitToSelect();
}

void EditorWindow::slotZoomTo100Percents()
{
    d->zoomPlusAction->setEnabled(true);
    d->zoomBar->setEnabled(true);
    d->zoomMinusAction->setEnabled(true);
    m_stackView->zoomTo100Percent();
}

void EditorWindow::slotZoomChanged(bool isMax, bool isMin, double zoom)
{
    //qCDebug(DIGIKAM_GENERAL_LOG) << "EditorWindow::slotZoomChanged";
    d->zoomPlusAction->setEnabled(!isMax);
    d->zoomMinusAction->setEnabled(!isMin);

    double zmin = m_stackView->zoomMin();
    double zmax = m_stackView->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);
}

void EditorWindow::slotToggleOffFitToWindow()
{
    d->zoomFitToWindowAction->blockSignals(true);
    d->zoomFitToWindowAction->setChecked(false);
    d->zoomFitToWindowAction->blockSignals(false);
}

void EditorWindow::loadImagePlugins()
{
    if (d->imagepluginsActionCollection)
    {
        d->imagepluginsActionCollection->clear();
        delete d->imagepluginsActionCollection;
    }

    d->imagepluginsActionCollection = new KActionCollection(dynamic_cast<QObject*>(this));

    QList<ImagePlugin*> pluginList = m_imagePluginLoader->pluginList();
    qDebug() << "Got total of " << pluginList.size() << " image plugins";

    foreach(ImagePlugin* const plugin, pluginList)
    {
        if (plugin)
        {
            guiFactory()->addClient(plugin);
            plugin->setEnabledSelectionActions(false);
            qDebug() << "loading plugin: " << plugin->componentName();
            // add actions to imagepluginsActionCollection
            QString categoryStr = plugin->actionCategory();

            if (categoryStr != QLatin1String("__INVALID__") && !categoryStr.isEmpty())
            {
                qDebug() << "Adding to category " << categoryStr;
                KActionCategory* const category = new KActionCategory(categoryStr, d->imagepluginsActionCollection);

                foreach(QAction* const action, plugin->actionCollection()->actions())
                {
                    qDebug() << "  Action: " << action->objectName();
                    category->addAction(action->objectName(), action);
                }
            }
            else
            {
                qDebug() << "Adding non-categorized plugins:";
                foreach(QAction* const action, plugin->actionCollection()->actions())
                {
                    qDebug() << "  " << action->objectName();
                    d->imagepluginsActionCollection->addAction(action->objectName(), action);
                }
            }
        }
        else
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid plugin to add!";
        }
    }

    // load imagepluginsActionCollection settings
    d->imagepluginsActionCollection->readSettings();
}

void EditorWindow::unLoadImagePlugins()
{
    if (d->imagepluginsActionCollection)
    {
        d->imagepluginsActionCollection->clear();
        delete d->imagepluginsActionCollection;
    }

    QList<ImagePlugin*> pluginList = m_imagePluginLoader->pluginList();

    foreach(ImagePlugin* const plugin, pluginList)
    {
        if (plugin)
        {
            guiFactory()->removeClient(plugin);
            plugin->setEnabledSelectionActions(false);
        }
    }
}

void EditorWindow::readStandardSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    // Restore Canvas layout
    if (group.hasKey(d->configVerticalSplitterSizesEntry) && m_vSplitter)
    {
        QByteArray state;
        state = group.readEntry(d->configVerticalSplitterStateEntry, state);
        m_vSplitter->restoreState(QByteArray::fromBase64(state));
    }

    // Restore full screen Mode
    readFullScreenSettings(group);

    // Restore Auto zoom action
    bool autoZoom = group.readEntry(d->configAutoZoomEntry, true);

    if (autoZoom)
    {
        d->zoomFitToWindowAction->trigger();
    }

    slotSetUnderExposureIndicator(group.readEntry(d->configUnderExposureIndicatorEntry, false));
    slotSetOverExposureIndicator(group.readEntry(d->configOverExposureIndicatorEntry, false));
    d->previewToolBar->readSettings(group);
}

void EditorWindow::applyStandardSettings()
{
    applyColorManagementSettings();
    d->toolIface->updateICCSettings();

    applyIOSettings();

    // -- GUI Settings -------------------------------------------------------

    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());

    d->legacyUpdateSplitterState(group);
    m_splitter->restoreState(group);
    readFullScreenSettings(group);

    slotThemeChanged();

    // -- Exposure Indicators Settings ---------------------------------------

    d->exposureSettings->underExposureColor    = group.readEntry(d->configUnderExposureColorEntry,    QColor(Qt::white));
    d->exposureSettings->underExposurePercent  = group.readEntry(d->configUnderExposurePercentsEntry, 1.0);
    d->exposureSettings->overExposureColor     = group.readEntry(d->configOverExposureColorEntry,     QColor(Qt::black));
    d->exposureSettings->overExposurePercent   = group.readEntry(d->configOverExposurePercentsEntry,  1.0);
    d->exposureSettings->exposureIndicatorMode = group.readEntry(d->configExpoIndicatorModeEntry,     true);
    d->toolIface->updateExposureSettings();

    // -- Metadata Settings --------------------------------------------------

    MetadataSettingsContainer writeSettings = MetadataSettings::instance()->settings();
    m_setExifOrientationTag                 = writeSettings.exifSetOrientation;
    m_canvas->setExifOrient(writeSettings.exifRotate);
}

void EditorWindow::applyIOSettings()
{
    // -- JPEG, PNG, TIFF JPEG2000 files format settings --------------------------------------

    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());

    m_IOFileSettings->JPEGCompression     = JPEGSettings::convertCompressionForLibJpeg(group.readEntry(d->configJpegCompressionEntry, 75));

    m_IOFileSettings->JPEGSubSampling     = group.readEntry(d->configJpegSubSamplingEntry, 1);  // Medium subsampling

    m_IOFileSettings->PNGCompression      = PNGSettings::convertCompressionForLibPng(group.readEntry(d->configPngCompressionEntry, 1));

    // TIFF compression setting.
    m_IOFileSettings->TIFFCompression     = group.readEntry(d->configTiffCompressionEntry, false);

    // JPEG2000 quality slider settings : 1 - 100
    m_IOFileSettings->JPEG2000Compression = group.readEntry(d->configJpeg2000CompressionEntry, 100);

    // JPEG2000 LossLess setting.
    m_IOFileSettings->JPEG2000LossLess    = group.readEntry(d->configJpeg2000LossLessEntry, true);

    // PGF quality slider settings : 1 - 9
    m_IOFileSettings->PGFCompression      = group.readEntry(d->configPgfCompressionEntry, 3);

    // PGF LossLess setting.
    m_IOFileSettings->PGFLossLess         = group.readEntry(d->configPgfLossLessEntry, true);

    // -- RAW images decoding settings ------------------------------------------------------

    m_IOFileSettings->useRAWImport = group.readEntry(d->configUseRawImportToolEntry, false);
    m_IOFileSettings->rawDecodingSettings.rawPrm.readSettings(group);

    // Raw Color Management settings:
    // If digiKam Color Management is enabled, no need to correct color of decoded RAW image,
    // else, sRGB color workspace will be used.

    ICCSettingsContainer settings = IccSettings::instance()->settings();

    if (settings.enableCM)
    {
        if (settings.defaultUncalibratedBehavior & ICCSettingsContainer::AutomaticColors)
        {
            m_IOFileSettings->rawDecodingSettings.rawPrm.outputColorSpace = RawDecodingSettings::CUSTOMOUTPUTCS;
            m_IOFileSettings->rawDecodingSettings.rawPrm.outputProfile    = settings.workspaceProfile;
        }
        else
        {
            m_IOFileSettings->rawDecodingSettings.rawPrm.outputColorSpace = RawDecodingSettings::RAWCOLOR;
        }
    }
    else
    {
        m_IOFileSettings->rawDecodingSettings.rawPrm.outputColorSpace = RawDecodingSettings::SRGB;
    }
}

void EditorWindow::applyColorManagementSettings()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    d->toolIface->updateICCSettings();
    m_canvas->setICCSettings(settings);

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedView);
    setColorManagedViewIndicatorToolTip(settings.enableCM, settings.useManagedView);
    d->viewCMViewAction->blockSignals(false);

    d->viewSoftProofAction->setEnabled(settings.enableCM && !settings.defaultProofProfile.isEmpty());
    d->softProofOptionsAction->setEnabled(settings.enableCM);
}

void EditorWindow::saveStandardSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    group.writeEntry(d->configAutoZoomEntry, d->zoomFitToWindowAction->isChecked());
    m_splitter->saveState(group);

    if (m_vSplitter)
    {
        group.writeEntry(d->configVerticalSplitterStateEntry, m_vSplitter->saveState().toBase64());
    }

    group.writeEntry("Show Thumbbar", thumbBar()->shouldBeVisible());
    group.writeEntry(d->configUnderExposureIndicatorEntry, d->exposureSettings->underExposureIndicator);
    group.writeEntry(d->configOverExposureIndicatorEntry,  d->exposureSettings->overExposureIndicator);
    d->previewToolBar->writeSettings(group);
    config->sync();
}

/** Method used by Editor Tools. Only tools based on imageregionwidget support zoomming.
    TODO: Fix this behavior when editor tool preview widgets will be factored.
 */
void EditorWindow::toggleZoomActions(bool val)
{
    d->zoomMinusAction->setEnabled(val);
    d->zoomPlusAction->setEnabled(val);
    d->zoomTo100percents->setEnabled(val);
    d->zoomFitToWindowAction->setEnabled(val);
    d->zoomBar->setEnabled(val);
}

void EditorWindow::readSettings()
{
    readStandardSettings();
}

void EditorWindow::saveSettings()
{
    saveStandardSettings();
}

void EditorWindow::toggleActions(bool val)
{
    toggleStandardActions(val);
}

void EditorWindow::toggleStandardActions(bool val)
{
    d->zoomFitToSelectAction->setEnabled(val);
    toggleZoomActions(val);

    m_forwardAction->setEnabled(val);
    m_backwardAction->setEnabled(val);
    m_firstAction->setEnabled(val);
    m_lastAction->setEnabled(val);
    d->rotateLeftAction->setEnabled(val);
    d->rotateRightAction->setEnabled(val);
    d->flipHorizAction->setEnabled(val);
    d->flipVertAction->setEnabled(val);
    d->filePrintAction->setEnabled(val);
    m_selectToolsAction->setEnabled(val);
    m_fileDeleteAction->setEnabled(val);
    m_saveAsAction->setEnabled(val);
    m_exportAction->setEnabled(val);
    d->selectAllAction->setEnabled(val);
    d->selectNoneAction->setEnabled(val);
    d->slideShowAction->setEnabled(val);

    // these actions are special: They are turned off if val is false,
    // but if val is true, they may be turned on or off.
    if (val)
    {
        // Update actions by retrieving current values
        slotUndoStateChanged();
    }
    else
    {
        m_openVersionAction->setEnabled(false);
        m_revertAction->setEnabled(false);
        m_saveAction->setEnabled(false);
        m_saveCurrentVersionAction->setEnabled(false);
        m_saveNewVersionAction->setEnabled(false);
        m_discardChangesAction->setEnabled(false);
        m_undoAction->setEnabled(false);
        m_redoAction->setEnabled(false);
    }

    QList<ImagePlugin*> pluginList = m_imagePluginLoader->pluginList();

    foreach(ImagePlugin* const plugin, pluginList)
    {
        if (plugin)
        {
            plugin->setEnabledActions(val);
        }
    }
}

void EditorWindow::toggleNonDestructiveActions()
{
    m_saveAction->setVisible(!m_nonDestructive);
    m_saveAsAction->setVisible(!m_nonDestructive);
    m_revertAction->setVisible(!m_nonDestructive);

    m_openVersionAction->setVisible(m_nonDestructive);
    m_saveCurrentVersionAction->setVisible(m_nonDestructive);
    m_saveNewVersionAction->setVisible(m_nonDestructive);
    m_exportAction->setVisible(m_nonDestructive);
    m_discardChangesAction->setVisible(m_nonDestructive);
}

void EditorWindow::toggleToolActions(EditorTool* tool)
{
    if (tool)
    {
        m_applyToolAction->setText(tool->toolSettings()->button(EditorToolSettings::Ok)->text());
        m_applyToolAction->setIcon(tool->toolSettings()->button(EditorToolSettings::Ok)->icon());
        m_applyToolAction->setToolTip(tool->toolSettings()->button(EditorToolSettings::Ok)->toolTip());

        m_closeToolAction->setText(tool->toolSettings()->button(EditorToolSettings::Cancel)->text());
        m_closeToolAction->setIcon(tool->toolSettings()->button(EditorToolSettings::Cancel)->icon());
        m_closeToolAction->setToolTip(tool->toolSettings()->button(EditorToolSettings::Cancel)->toolTip());
    }

    m_applyToolAction->setVisible(tool);
    m_closeToolAction->setVisible(tool);
}

void EditorWindow::slotLoadingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress * 100.0));
}

void EditorWindow::slotSavingProgress(const QString&, float progress)
{
    m_nameLabel->setProgressValue((int)(progress * 100.0));

    if (m_savingProgressDialog)
    {
        m_savingProgressDialog->setValue((int)(progress * 100.0));
    }
}

void EditorWindow::execSavingProgressDialog()
{
    if (m_savingProgressDialog)
    {
        return;
    }

    m_savingProgressDialog = new QProgressDialog(this);
    m_savingProgressDialog->setWindowTitle(i18n("Saving image..."));
    m_savingProgressDialog->setLabelText(i18n("Please wait for the image to be saved..."));
    m_savingProgressDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_savingProgressDialog->setAutoClose(true);
    m_savingProgressDialog->setMinimumDuration(1000);
    m_savingProgressDialog->setMaximum(100);
    // we must enter a fully modal dialog, no QEventLoop is sufficient for KWin to accept longer waiting times
    m_savingProgressDialog->setModal(true);
    m_savingProgressDialog->exec();
}

bool EditorWindow::promptForOverWrite()
{

    QUrl destination = saveDestinationUrl();

    if (destination.isLocalFile())
    {

        QFileInfo fi(m_canvas->currentImageFilePath());
        QString warnMsg(i18n("About to overwrite file \"%1\"\nAre you sure?", QDir::toNativeSeparators(fi.fileName())));

        return (DMessageBox::showContinueCancel(QMessageBox::Warning,
                                                this,
                                                i18n("Warning"),
                                                warnMsg,
                                                QLatin1String("editorWindowSaveOverwrite"))
                ==  QMessageBox::Yes);

    }
    else
    {
        // in this case kio handles the overwrite request
        return true;
    }

}

void EditorWindow::slotUndoStateChanged()
{
    UndoState state = m_canvas->interface()->undoState();

    // RAW conversion qualifies as a "non-undoable" action
    // You can save as new version, but cannot undo or revert
    m_undoAction->setEnabled(state.hasUndo);
    m_redoAction->setEnabled(state.hasRedo);
    m_revertAction->setEnabled(state.hasUndoableChanges);

    m_saveAction->setEnabled(state.hasChanges);
    m_saveCurrentVersionAction->setEnabled(state.hasChanges);
    m_saveNewVersionAction->setEnabled(state.hasChanges);
    m_discardChangesAction->setEnabled(state.hasUndoableChanges);

    m_openVersionAction->setEnabled(hasOriginalToRestore());
}

bool EditorWindow::hasOriginalToRestore()
{
    return m_canvas->interface()->getResolvedInitialHistory().hasOriginalReferredImage();
}

DImageHistory EditorWindow::resolvedImageHistory(const DImageHistory& history)
{
    // simple, database-less version
    DImageHistory r = history;
    QList<DImageHistory::Entry>::iterator it;

    for (it = r.entries().begin(); it != r.entries().end(); ++it)
    {
        QList<HistoryImageId>::iterator hit;

        for (hit = it->referredImages.begin(); hit != it->referredImages.end();)
        {
            QFileInfo info(hit->m_filePath + QLatin1Char('/') + hit->m_fileName);

            if (!info.exists())
            {
                hit = it->referredImages.erase(hit);
            }
            else
            {
                ++hit;
            }
        }
    }

    return r;
}

bool EditorWindow::promptUserSave(const QUrl& url, SaveAskMode mode, bool allowCancel)
{
    if (d->currentWindowModalDialog)
    {
        d->currentWindowModalDialog->reject();
    }

    if (m_canvas->interface()->undoState().hasUndoableChanges)
    {
        // if window is minimized, show it
        if (isMinimized())
        {
            KWindowSystem::unminimizeWindow(winId());
        }

        bool shallSave    = true;
        bool shallDiscard = false;
        bool newVersion   = false;

        if (mode == AskIfNeeded)
        {
            if (m_nonDestructive)
            {
                if (versionManager()->settings().editorClosingMode == VersionManagerSettings::AutoSave)
                {
                    shallSave = true;
                }
                else
                {
                    QPointer<VersioningPromptUserSaveDialog> dialog = new VersioningPromptUserSaveDialog(this);
                    dialog->exec();

                    if (!dialog)
                    {
                        return false;
                    }

                    shallSave    = dialog->shallSave() || dialog->newVersion();
                    shallDiscard = dialog->shallDiscard();
                    newVersion   = dialog->newVersion();
                }
            }
            else
            {
                QString boxMessage;
                boxMessage = i18nc("@info",
                                   "<qt>The image <b>%1</b> has been modified.<br>"
                                   "Do you want to save it?</qt>", url.fileName());

                int result;

                if (allowCancel)
                {
                    result = QMessageBox::warning(this, qApp->applicationName(), boxMessage,
                                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                }
                else
                {
                    result = QMessageBox::warning(this, qApp->applicationName(), boxMessage,
                                                  QMessageBox::Save | QMessageBox::Discard);
                }

                shallSave    = (result == QMessageBox::Save);
                shallDiscard = (result == QMessageBox::Discard);
            }
        }

        if (shallSave)
        {
            bool saving = false;

            switch (mode)
            {
                case AskIfNeeded:

                    if (m_nonDestructive)
                    {
                        if (newVersion)
                        {
                            saving = saveNewVersion();
                        }
                        else
                        {
                            // will know on its own if new version is required
                            saving = saveCurrentVersion();
                        }
                    }
                    else
                    {
                        if (m_canvas->isReadOnly())
                        {
                            saving = saveAs();
                        }
                        else if (promptForOverWrite())
                        {
                            saving = save();
                        }
                    }

                    break;

                case OverwriteWithoutAsking:

                    if (m_nonDestructive)
                    {
                        if (newVersion)
                        {
                            saving = saveNewVersion();
                        }
                        else
                        {
                            // will know on its own if new version is required
                            saving = saveCurrentVersion();
                        }
                    }
                    else
                    {
                        if (m_canvas->isReadOnly())
                        {
                            saving = saveAs();
                        }
                        else
                        {
                            saving = save();
                        }
                    }

                    break;

                case AlwaysSaveAs:

                    if (m_nonDestructive)
                    {
                        saving = saveNewVersion();
                    }
                    else
                    {
                        saving = saveAs();
                    }

                    break;
            }

            // save and saveAs return false if they were canceled and did not enter saving at all
            // In this case, do not call enterWaitingLoop because quitWaitingloop will not be called.
            if (saving)
            {
                // Waiting for asynchronous image file saving operation running in separate thread.
                m_savingContext.synchronizingState = SavingContext::SynchronousSaving;
                enterWaitingLoop();
                m_savingContext.synchronizingState = SavingContext::NormalSaving;
                return m_savingContext.synchronousSavingResult;
            }
            else
            {
                return false;
            }
        }
        else if (shallDiscard)
        {
            // Discard
            m_saveAction->setEnabled(false);
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::waitForSavingToComplete()
{
    // avoid reentrancy - return false means we have reentered the loop already.
    if (m_savingContext.synchronizingState == SavingContext::SynchronousSaving)
    {
        return false;
    }

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        // Waiting for asynchronous image file saving operation running in separate thread.
        m_savingContext.synchronizingState = SavingContext::SynchronousSaving;

        enterWaitingLoop();
        m_savingContext.synchronizingState = SavingContext::NormalSaving;
    }

    return true;
}

void EditorWindow::enterWaitingLoop()
{
    //d->waitingLoop->exec(QEventLoop::ExcludeUserInputEvents);

    execSavingProgressDialog();
}

void EditorWindow::quitWaitingLoop()
{
    //d->waitingLoop->quit();

    if (m_savingProgressDialog)
    {
        m_savingProgressDialog->close();
    }
}

void EditorWindow::slotSelected(bool val)
{
    // Update menu actions.
    d->cropAction->setEnabled(val);
    d->zoomFitToSelectAction->setEnabled(val);
    d->copyAction->setEnabled(val);

    QList<ImagePlugin*> pluginList = m_imagePluginLoader->pluginList();

    foreach(ImagePlugin* const plugin, pluginList)
    {
        if (plugin)
        {
            plugin->setEnabledSelectionActions(val);
        }
    }

    QRect sel = m_canvas->getSelectedArea();
    // Update histogram into sidebar.
    emit signalSelectionChanged(sel);

    // Update status bar
    if (val)
    {
        slotSelectionSetText(sel);
    }
    else
    {
        setToolInfoMessage(i18n("No selection"));
    }
}

void EditorWindow::slotPrepareToLoad()
{
    // Disable actions as appropriate during loading
    emit signalNoCurrentItem();
    unsetCursor();
    m_animLogo->stop();
    toggleActions(false);
    slotUpdateItemInfo();
}

void EditorWindow::slotLoadingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    toggleActions(false);
    m_animLogo->start();
    m_nameLabel->progressBarMode(StatusProgressBar::ProgressBarMode, i18n("Loading: "));
}

void EditorWindow::slotLoadingFinished(const QString& /*filename*/, bool success)
{
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();

    // Enable actions as appropriate after loading
    // No need to re-enable image properties sidebar here, it's will be done
    // automatically by a signal from canvas
    toggleActions(success);
    unsetCursor();
    m_animLogo->stop();

    if (success)
    {
        colorManage();

        // Set a history which contains all available files as referredImages
        DImageHistory resolved = resolvedImageHistory(m_canvas->interface()->getInitialImageHistory());
        m_canvas->interface()->setResolvedInitialHistory(resolved);
    }
}

void EditorWindow::resetOrigin()
{
    // With versioning, "only" resetting undo history does not work anymore
    // as we calculate undo state based on the initial history stored in the DImg
    resetOriginSwitchFile();
}

void EditorWindow::resetOriginSwitchFile()
{
    DImageHistory resolved = resolvedImageHistory(m_canvas->interface()->getImageHistory());
    m_canvas->interface()->switchToLastSaved(resolved);
}

void EditorWindow::colorManage()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    DImg image = m_canvas->currentImage();

    if (image.isNull())
    {
        return;
    }

    if (!IccManager::needsPostLoadingManagement(image))
    {
        return;
    }

    IccPostLoadingManager manager(image, m_canvas->currentImageFilePath());

    if (!manager.hasValidWorkspace())
    {
        QString message = i18n("Cannot open the specified working space profile (\"%1\"). "
                               "No color transformation will be applied. "
                               "Please check the color management "
                               "configuration in digiKam's setup.", IccSettings::instance()->settings().workspaceProfile);
        QMessageBox::information(this, qApp->applicationName(), message);
    }

    // Show dialog and get transform from user choice
    IccTransform trans = manager.postLoadingManage(this);
    // apply transform in thread.
    // Do _not_ test for willHaveEffect() here - there are more side effects when calling this method
    m_canvas->applyTransform(trans);
    slotUpdateItemInfo();
}

void EditorWindow::slotNameLabelCancelButtonPressed()
{
    // If we saving an image...
    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        m_savingContext.abortingSaving = true;
        m_canvas->abortSaving();
    }

    // If we preparing SlideShow...
    m_cancelSlideShow = true;
}

void EditorWindow::slotFileOriginChanged(const QString&)
{
    // implemented in subclass
}

bool EditorWindow::saveOrSaveAs()
{
    if (m_canvas->isReadOnly())
    {
        return saveAs();
    }
    else if (promptForOverWrite())
    {
        return save();
    }

    return false;
}

void EditorWindow::slotSavingStarted(const QString& /*filename*/)
{
    setCursor(Qt::WaitCursor);
    m_animLogo->start();

    // Disable actions as appropriate during saving
    emit signalNoCurrentItem();
    toggleActions(false);

    m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, i18n("Saving: "));
}

void EditorWindow::slotSavingFinished(const QString& filename, bool success)
{
    Q_UNUSED(filename);
    qCDebug(DIGIKAM_GENERAL_LOG) << filename << success << (m_savingContext.savingState != SavingContext::SavingStateNone);

    // only handle this if we really wanted to save a file...
    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        m_savingContext.executedOperation = m_savingContext.savingState;
        m_savingContext.savingState = SavingContext::SavingStateNone;

        if (!success)
        {
            if (!m_savingContext.abortingSaving)
            {
                QMessageBox::critical(this, qApp->applicationName(),
                                      i18n("Failed to save file\n\"%1\"\nto\n\"%2\".",
                                           m_savingContext.destinationURL.fileName(),
                                           m_savingContext.destinationURL.toLocalFile()));
            }

            finishSaving(false);
            return;
        }

        /*
         *            / -> moveLocalFile()                    \
         * moveFile()                                           ->     movingSaveFileFinished()
         *            \ -> KIO::move() -> slotKioMoveFinished /        |              |
         *
         *                                                    finishSaving(true)  save...IsComplete()
         */
        moveFile();

    }
    else
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Why was slotSavingFinished called if we did not want to save a file?";
    }
}

void EditorWindow::movingSaveFileFinished(bool successful)
{
    if (!successful)
    {
        finishSaving(false);
        return;
    }

    // now that we know the real destination file name, pass it to be recorded in image history
    m_canvas->interface()->setLastSaved(m_savingContext.destinationURL.toLocalFile());

    // remove image from cache since it has changed
    LoadingCacheInterface::fileChanged(m_savingContext.destinationURL.toLocalFile());
    ThumbnailLoadThread::deleteThumbnail(m_savingContext.destinationURL.toLocalFile());

    // restore state of disabled actions. saveIsComplete can start any other task
    // (loading!) which might itself in turn change states
    finishSaving(true);

    switch (m_savingContext.executedOperation)
    {
        case SavingContext::SavingStateNone:
            break;

        case SavingContext::SavingStateSave:
            saveIsComplete();
            break;

        case SavingContext::SavingStateSaveAs:
            saveAsIsComplete();
            break;

        case SavingContext::SavingStateVersion:
            saveVersionIsComplete();
            break;
    }

    // Take all actions necessary to update information and re-enable sidebar
    slotChanged();
}

void EditorWindow::finishSaving(bool success)
{
    m_savingContext.synchronousSavingResult = success;

    delete m_savingContext.saveTempFile;
    m_savingContext.saveTempFile = 0;

    // Exit of internal Qt event loop to unlock promptUserSave() method.
    if (m_savingContext.synchronizingState == SavingContext::SynchronousSaving)
    {
        quitWaitingLoop();
    }

    // Enable actions as appropriate after saving
    toggleActions(true);
    unsetCursor();
    m_animLogo->stop();

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    /*if (m_savingProgressDialog)
    {
        m_savingProgressDialog->close();
    }*/

    // On error, continue using current image
    if (!success)
    {
        /* Why this?
         * m_canvas->switchToLastSaved(m_savingContext.srcURL.toLocalFile());*/
    }
}

void EditorWindow::setupTempSaveFile(const QUrl& url)
{
    // if the destination url is on local file system, try to set the temp file
    // location to the destination folder, otherwise use a local default
    QString tempDir;

    if (url.isLocalFile())
    {
#ifdef _WIN32
        QUrl parent(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path());
        tempDir = parent.toLocalFile();
#else
        tempDir = url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path();
#endif
    }
    else
    {
        tempDir = QDir::tempPath();
    }

    QString path = url.path();
    int lastDot  = path.lastIndexOf(QLatin1Char('.'));
    QString suffix = path.mid(lastDot + 1);

    // use magic file extension which tells the digikamalbums ioslave to ignore the file
    m_savingContext.saveTempFile = new SafeTemporaryFile(tempDir + QLatin1String("/EditorWindow-XXXXXX.digikamtempfile.") + suffix);
    m_savingContext.saveTempFile->setAutoRemove(false);

    if (!m_savingContext.saveTempFile->open())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Could not open a temporary file in the folder \"%1\": %2 (%3)",
                                   QDir::toNativeSeparators(tempDir), m_savingContext.saveTempFile->errorString(),
                                   m_savingContext.saveTempFile->error()));
        return;
    }

    m_savingContext.saveTempFileName = m_savingContext.saveTempFile->fileName();
    delete m_savingContext.saveTempFile;
    m_savingContext.saveTempFile = 0;
}

void EditorWindow::startingSave(const QUrl& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "startSaving url = " << url;

    // avoid any reentrancy. Should be impossible anyway since actions will be disabled.
    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        return;
    }

    m_savingContext = SavingContext();

    if (!checkPermissions(url))
    {
        return;
    }

    setupTempSaveFile(url);

    m_savingContext.srcURL             = url;
    m_savingContext.destinationURL     = m_savingContext.srcURL;
    m_savingContext.destinationExisted = true;
    m_savingContext.originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext.format             = m_savingContext.originalFormat;
    m_savingContext.abortingSaving     = false;
    m_savingContext.savingState        = SavingContext::SavingStateSave;
    m_savingContext.executedOperation  = SavingContext::SavingStateNone;

    m_canvas->interface()->saveAs(m_savingContext.saveTempFileName, m_IOFileSettings,
                                  m_setExifOrientationTag && m_canvas->exifRotated(), m_savingContext.format,
                                  m_savingContext.destinationURL.toLocalFile());
}

bool EditorWindow::showFileSaveDialog(const QUrl& initialUrl, QUrl& newURL)
{
    QString all;
    QStringList list                       = supportedImageMimeTypes(QIODevice::WriteOnly, all);
    QFileDialog* const imageFileSaveDialog = new QFileDialog(this);
    imageFileSaveDialog->setWindowTitle(i18n("New Image File Name"));
    imageFileSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    imageFileSaveDialog->setDirectoryUrl(initialUrl);
    imageFileSaveDialog->setNameFilters(list);
    imageFileSaveDialog->setOption(QFileDialog::DontUseNativeDialog);
    imageFileSaveDialog->setFileMode(QFileDialog::AnyFile);

    // restore old settings for the dialog
    KSharedConfig::Ptr config         = KSharedConfig::openConfig();
    KConfigGroup group                = config->group(configGroupName());
    const QString optionLastExtension = QLatin1String("LastSavedImageExtension");
    QString ext                       = group.readEntry(optionLastExtension, "png");
    
    Q_FOREACH(QString s, list)
    {
        if (s.contains(QString::fromLatin1("*.%1").arg(ext)))
        {
            imageFileSaveDialog->selectNameFilter(s);
        }
    }

    // adjust extension of proposed filename
    QString fileName                  = initialUrl.fileName();

    if (!fileName.isNull())
    {
        int lastDot              = fileName.lastIndexOf(QLatin1Char('.'));
        QString completeBaseName = (lastDot == -1) ? fileName : fileName.left(lastDot);
        fileName                 = completeBaseName + QLatin1Char('.') + ext;
    }

    if (!fileName.isNull())
    {
        imageFileSaveDialog->selectFile(fileName);
    }

    // Start dialog and check if canceled.
    int result;

    if (d->currentWindowModalDialog)
    {
        // go application-modal - we will create utter confusion if descending into more than one window-modal dialog
        imageFileSaveDialog->setModal(true);
        result = imageFileSaveDialog->exec();
    }
    else
    {
        imageFileSaveDialog->setWindowModality(Qt::WindowModal);
        d->currentWindowModalDialog = imageFileSaveDialog;
        result                      = imageFileSaveDialog->exec();
        d->currentWindowModalDialog = 0;
    }

    if (result != QFileDialog::Accepted || !imageFileSaveDialog)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "File Save Dialog rejected";
        return false;
    }

    QList<QUrl> urls = imageFileSaveDialog->selectedUrls();

    if (urls.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "no target url";
        return false;
    }
        
    newURL = urls.first();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Writing file to " << newURL;

    //-- Show Settings Dialog ----------------------------------------------

    const QString configShowImageSettingsDialog = QLatin1String("ShowImageSettingsDialog");
    bool showDialog                             = group.readEntry(configShowImageSettingsDialog, true);
    FileSaveOptionsBox* const options           = new FileSaveOptionsBox();
    
    if (showDialog && options->discoverFormat(newURL.fileName(), DImg::NONE) != DImg::NONE)
    {
        FileSaveOptionsDlg* const fileSaveOptionsDialog = new FileSaveOptionsDlg(this, options);
        options->setImageFileFormat(newURL.fileName());

        if (d->currentWindowModalDialog)
        {
            // go application-modal - we will create utter confusion if descending into more than one window-modal dialog
            fileSaveOptionsDialog->setModal(true);
            result = fileSaveOptionsDialog->exec();
        }
        else
        {
            fileSaveOptionsDialog->setWindowModality(Qt::WindowModal);
            d->currentWindowModalDialog = fileSaveOptionsDialog;
            result                      = fileSaveOptionsDialog->exec();
            d->currentWindowModalDialog = 0;
        }

        if (result != QDialog::Accepted || !fileSaveOptionsDialog)
        {
            return false;
        }
    }

    // write settings to config
    options->applySettings();
    // read settings from config to local container
    applyIOSettings();

    // select the format to save the image with
    m_savingContext.format = selectValidSavingFormat(newURL);

    if (m_savingContext.format.isNull())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Unable to determine the format to save the target image with."));
        return false;
    }

    if (!newURL.isValid())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Cannot Save: Found file path <b>%1</b> is invalid.", newURL.toDisplayString()));
        qCWarning(DIGIKAM_GENERAL_LOG) << "target URL is not valid !";
        return false;
    }

    group.writeEntry(optionLastExtension, m_savingContext.format);
    config->sync();

    return true;
}

QString EditorWindow::selectValidSavingFormat(const QUrl& targetUrl)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Trying to find a saving format from targetUrl = " << targetUrl;

    // build a list of valid types
    QString all;
    supportedImageMimeTypes(QIODevice::WriteOnly, all);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Qt Offered types: " << all;

    QStringList validTypes = all.split(QLatin1String("*."), QString::SkipEmptyParts);
    validTypes.replaceInStrings(QLatin1String(" "), QString());
    
    qCDebug(DIGIKAM_GENERAL_LOG) << "Writable formats: " << validTypes;

    // determine the format to use the format provided in the filename

    QString suffix;

    if (targetUrl.isLocalFile())
    {
        // for local files QFileInfo can be used
        QFileInfo fi(targetUrl.toLocalFile());
        suffix = fi.suffix();
        qCDebug(DIGIKAM_GENERAL_LOG) << "Possible format from local file: " << suffix;
    }
    else
    {
        // for remote files string manipulation is needed unfortunately
        QString fileName         = targetUrl.fileName();
        const int periodLocation = fileName.lastIndexOf(QLatin1Char('.'));

        if (periodLocation >= 0)
        {
            suffix = fileName.right(fileName.size() - periodLocation - 1);
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Possible format from remote file: " << suffix;
    }

    if (!suffix.isEmpty() && validTypes.contains(suffix, Qt::CaseInsensitive))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Using format from target url " << suffix;
        return suffix;
    }

    // another way to determine the format is to use the original file
    {
        QString originalFormat = QString::fromUtf8(QImageReader::imageFormat(m_savingContext.srcURL.toLocalFile()));

        if (validTypes.contains(originalFormat, Qt::CaseInsensitive))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Using format from original file: " << originalFormat;
            return originalFormat;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "No suitable format found";

    return QString();
}

bool EditorWindow::startingSaveAs(const QUrl& url)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "startSavingAs called";

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        return false;
    }

    m_savingContext        = SavingContext();
    m_savingContext.srcURL = url;
    QUrl suggested         = m_savingContext.srcURL;

    // Run dialog -------------------------------------------------------------------

    QUrl newURL;

    if (!showFileSaveDialog(suggested, newURL))
    {
        return false;
    }

    // if new and original URL are equal use save() ------------------------------

    QUrl currURL(m_savingContext.srcURL);
    currURL.setPath(QDir::cleanPath(currURL.path()));
    newURL.setPath(QDir::cleanPath(newURL.path()));

    if (currURL.matches(newURL, QUrl::None))
    {
        save();
        return false;
    }

    // Check for overwrite ----------------------------------------------------------

    QFileInfo fi(newURL.toLocalFile());
    m_savingContext.destinationExisted = fi.exists();

    if (m_savingContext.destinationExisted)
    {
        if (!checkOverwrite(newURL))
        {
            return false;
        }

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.
        if (!checkPermissions(newURL))
        {
            return false;
        }
    }

    // Now do the actual saving -----------------------------------------------------

    setupTempSaveFile(newURL);

    m_savingContext.destinationURL = newURL;
    m_savingContext.originalFormat = m_canvas->currentImageFileFormat();
    m_savingContext.savingState    = SavingContext::SavingStateSaveAs;
    m_savingContext.executedOperation = SavingContext::SavingStateNone;
    m_savingContext.abortingSaving = false;

    // in any case, destructive (Save as) or non (Export), mark as New Version
    m_canvas->interface()->setHistoryIsBranch(true);

    m_canvas->interface()->saveAs(m_savingContext.saveTempFileName, m_IOFileSettings,
                                  m_setExifOrientationTag && m_canvas->exifRotated(),
                                  m_savingContext.format.toLower(),
                                  m_savingContext.destinationURL.toLocalFile());

    return true;
}

bool EditorWindow::startingSaveCurrentVersion(const QUrl& url)
{
    return startingSaveVersion(url, false, false, QString());
}

bool EditorWindow::startingSaveNewVersion(const QUrl& url)
{
    return startingSaveVersion(url, true, false, QString());
}

bool EditorWindow::startingSaveNewVersionAs(const QUrl& url)
{
    return startingSaveVersion(url, true, true, QString());
}

bool EditorWindow::startingSaveNewVersionInFormat(const QUrl& url, const QString& format)
{
    return startingSaveVersion(url, true, false, format);
}

VersionFileOperation EditorWindow::saveVersionFileOperation(const QUrl& url, bool fork)
{
    DImageHistory resolvedHistory = m_canvas->interface()->getResolvedInitialHistory();
    DImageHistory history = m_canvas->interface()->getImageHistory();

    VersionFileInfo currentName(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path(), url.fileName(), m_canvas->currentImageFileFormat());
    return versionManager()->operation(fork ? VersionManager::NewVersionName : VersionManager::CurrentVersionName,
                                       currentName, resolvedHistory, history);
}

VersionFileOperation EditorWindow::saveAsVersionFileOperation(const QUrl& url, const QUrl& saveUrl, const QString& format)
{
    DImageHistory resolvedHistory = m_canvas->interface()->getResolvedInitialHistory();
    DImageHistory history         = m_canvas->interface()->getImageHistory();

    VersionFileInfo currentName(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path(), url.fileName(), m_canvas->currentImageFileFormat());
    VersionFileInfo saveLocation(saveUrl.adjusted(QUrl::RemoveFilename).path(), saveUrl.fileName(), format);
    return versionManager()->operationNewVersionAs(currentName, saveLocation, resolvedHistory, history);
}

VersionFileOperation EditorWindow::saveInFormatVersionFileOperation(const QUrl& url, const QString& format)
{
    DImageHistory resolvedHistory = m_canvas->interface()->getResolvedInitialHistory();
    DImageHistory history         = m_canvas->interface()->getImageHistory();

    VersionFileInfo currentName(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path(), url.fileName(), m_canvas->currentImageFileFormat());
    return versionManager()->operationNewVersionInFormat(currentName, format, resolvedHistory, history);
}

bool EditorWindow::startingSaveVersion(const QUrl& url, bool fork, bool saveAs, const QString& format)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Saving image" << url << "non-destructive, new version:"
             << fork << ", saveAs:" << saveAs << "format:" << format;

    if (m_savingContext.savingState != SavingContext::SavingStateNone)
    {
        return false;
    }

    m_savingContext                      = SavingContext();
    m_savingContext.versionFileOperation = saveVersionFileOperation(url, fork);
    m_canvas->interface()->setHistoryIsBranch(fork);

    if (saveAs)
    {
        QUrl suggested = m_savingContext.versionFileOperation.saveFile.fileUrl();
        QUrl selectedUrl;

        if (!showFileSaveDialog(suggested, selectedUrl))
        {
            return false;
        }

        m_savingContext.versionFileOperation = saveAsVersionFileOperation(url, selectedUrl, m_savingContext.format);
    }
    else if (!format.isNull())
    {
        m_savingContext.versionFileOperation = saveInFormatVersionFileOperation(url, format);
    }

    const QUrl newURL = m_savingContext.versionFileOperation.saveFile.fileUrl();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Writing file to " << newURL;

    if (!newURL.isValid())
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18nc("@info",
                                    "Cannot save file <b>%1</b> to "
                                    "the suggested version file name <b>%2</b>",
                                    url.fileName(),
                                    newURL.fileName()));
        qCWarning(DIGIKAM_GENERAL_LOG) << "target URL is not valid !";
        return false;
    }

    QFileInfo fi(newURL.toLocalFile());
    m_savingContext.destinationExisted = fi.exists();

    // Check for overwrite (saveAs only) --------------------------------------------

    if (m_savingContext.destinationExisted)
    {
        // So, should we refuse to overwrite the original?
        // It's a frontal crash againt non-destructive principles.
        // It is tempting to refuse, yet I think the user has to decide in the end
        /*QUrl currURL(m_savingContext.srcURL);
        currURL.cleanPath();
        newURL.cleanPath();
        if (currURL.equals(newURL))
        {
            ...
            return false;
        }*/

        // check for overwrite, unless the operation explicitly tells us to overwrite
        if (!(m_savingContext.versionFileOperation.tasks & VersionFileOperation::Replace) && 
            !checkOverwrite(newURL))
        {
            return false;
        }

        // There will be two message boxes if the file is not writable.
        // This may be controversial, and it may be changed, but it was a deliberate decision.
        if (!checkPermissions(newURL))
        {
            return false;
        }
    }

    setupTempSaveFile(newURL);

    m_savingContext.srcURL             = url;
    m_savingContext.destinationURL     = newURL;
    m_savingContext.originalFormat     = m_canvas->currentImageFileFormat();
    m_savingContext.format             = m_savingContext.versionFileOperation.saveFile.format;
    m_savingContext.abortingSaving     = false;
    m_savingContext.savingState        = SavingContext::SavingStateVersion;
    m_savingContext.executedOperation  = SavingContext::SavingStateNone;

    m_canvas->interface()->saveAs(m_savingContext.saveTempFileName, m_IOFileSettings,
                                  m_setExifOrientationTag && m_canvas->exifRotated(),
                                  m_savingContext.format.toLower(),
                                  m_savingContext.versionFileOperation);

    return true;
}

bool EditorWindow::checkPermissions(const QUrl& url)
{
    //TODO: Check that the permissions can actually be changed
    //      if write permissions are not available.

    QFileInfo fi(url.toLocalFile());

    if (fi.exists() && !fi.isWritable())
    {
        int result = QMessageBox::warning(this, i18n("Overwrite File?"),
                                          i18n("You do not have write permissions "
                                               "for the file named \"%1\". "
                                               "Are you sure you want "
                                               "to overwrite it?",
                                               url.fileName()),
                                          QMessageBox::Save | QMessageBox::Cancel);

        if (result != QMessageBox::Save)
        {
            return false;
        }
    }

    return true;
}

bool EditorWindow::checkOverwrite(const QUrl& url)
{
    int result = QMessageBox::warning(this, i18n("Overwrite File?"),
                                      i18n("A file named \"%1\" already "
                                           "exists. Are you sure you want "
                                           "to overwrite it?",
                                           url.fileName()),
                                      QMessageBox::Save | QMessageBox::Cancel);

    return (result == QMessageBox::Save);
}

bool EditorWindow::moveLocalFile(const QString& org, const QString& dst)
{
    QString sidecarOrg = DMetadata::sidecarFilePathForFile(org);
    QString source     = m_savingContext.srcURL.toLocalFile();

    if (QFileInfo(sidecarOrg).exists())
    {
        QString sidecarDst = DMetadata::sidecarFilePathForFile(dst);

        if (!FileOperation::localFileRename(source, sidecarOrg, sidecarDst))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to move sidecar file";
        }
    }

    if (!FileOperation::localFileRename(source, org, dst))
    {
        QMessageBox::critical(this, i18n("Error Saving File"),
                              i18n("Failed to overwrite original file"));
        return false;
    }

    return true;
}

void EditorWindow::moveFile()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << m_savingContext.destinationURL << m_savingContext.destinationURL.isLocalFile();

    // how to move a file depends on if the file is on a local system or not.
    if (m_savingContext.destinationURL.isLocalFile())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "moving a local file";

        if (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
        {
            // check if we need to move the current file to an intermediate name
            if (m_savingContext.versionFileOperation.tasks & VersionFileOperation::MoveToIntermediate)
            {
                //qCDebug(DIGIKAM_GENERAL_LOG) << "MoveToIntermediate: Moving " << m_savingContext.srcURL.toLocalFile() << "to" <<
                //       m_savingContext.versionFileOperation.intermediateForLoadedFile.filePath() <<
                moveLocalFile(m_savingContext.srcURL.toLocalFile(),
                              m_savingContext.versionFileOperation.intermediateForLoadedFile.filePath());

                LoadingCacheInterface::fileChanged(m_savingContext.destinationURL.toLocalFile());
                ThumbnailLoadThread::deleteThumbnail(m_savingContext.destinationURL.toLocalFile());
            }
        }

        bool moveSuccessful = moveLocalFile(m_savingContext.saveTempFileName,
                                            m_savingContext.destinationURL.toLocalFile());

        if (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
        {
            if (moveSuccessful &&
                m_savingContext.versionFileOperation.tasks & VersionFileOperation::SaveAndDelete)
            {
                QFile file(m_savingContext.versionFileOperation.loadedFile.filePath());
                file.remove();
            }
        }

        movingSaveFileFinished(moveSuccessful);
    }
    else
    {
        // for remote destinations use kio to move the temp file over there
        // do not care for versioning here, atm not supported

        qCDebug(DIGIKAM_GENERAL_LOG) << "moving a remote file via KIO";

        if (DMetadata::hasSidecar(m_savingContext.saveTempFileName))
        {
            KIO::move(DMetadata::sidecarUrl(m_savingContext.saveTempFileName),
                      DMetadata::sidecarUrl(m_savingContext.destinationURL));
        }

        KIO::CopyJob* const moveJob = KIO::move(QUrl::fromLocalFile(m_savingContext.saveTempFileName),
                                                m_savingContext.destinationURL);

        connect(moveJob, SIGNAL(result(KJob*)),
                this, SLOT(slotKioMoveFinished(KJob*)));
    }
}

void EditorWindow::slotKioMoveFinished(KJob* job)
{
    if (job->error())
    {
        QMessageBox::critical(this, i18n("Error Saving File"),
                              i18n("Failed to save file: %1", job->errorString()));
    }

    movingSaveFileFinished(!job->error());
}

void EditorWindow::slotDiscardChanges()
{
    m_canvas->interface()->rollbackToOrigin();
}

void EditorWindow::slotOpenOriginal()
{
    // no-op in this base class
}

void EditorWindow::slotColorManagementOptionsChanged()
{
    applyColorManagementSettings();
    applyIOSettings();
}

void EditorWindow::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedView;
    IccSettings::instance()->setUseManagedView(cmv);
}

void EditorWindow::setColorManagedViewIndicatorToolTip(bool available, bool cmv)
{
    QString tooltip;

    if (available)
    {
        if (cmv)
        {
            tooltip = i18n("Color-Managed View is enabled.");
        }
        else
        {
            tooltip = i18n("Color-Managed View is disabled.");
        }
    }
    else
    {
        tooltip = i18n("Color Management is not configured, so the Color-Managed View is not available.");
    }

    d->cmViewIndicator->setToolTip(tooltip);
}

void EditorWindow::slotSoftProofingOptions()
{
    // Adjusts global settings
    QPointer<SoftProofDialog> dlg = new SoftProofDialog(this);
    dlg->exec();

    d->viewSoftProofAction->setChecked(dlg->shallEnableSoftProofView());
    slotUpdateSoftProofingState();
    delete dlg;
}

void EditorWindow::slotUpdateSoftProofingState()
{
    bool on = d->viewSoftProofAction->isChecked();
    m_canvas->setSoftProofingEnabled(on);
    d->toolIface->updateICCSettings();
}

void EditorWindow::slotSetUnderExposureIndicator(bool on)
{
    d->exposureSettings->underExposureIndicator = on;
    d->toolIface->updateExposureSettings();
    d->viewUnderExpoAction->setChecked(on);
    setUnderExposureToolTip(on);
}

void EditorWindow::setUnderExposureToolTip(bool on)
{
    d->underExposureIndicator->setToolTip(
        on ? i18n("Under-Exposure indicator is enabled")
           : i18n("Under-Exposure indicator is disabled"));
}

void EditorWindow::slotSetOverExposureIndicator(bool on)
{
    d->exposureSettings->overExposureIndicator = on;
    d->toolIface->updateExposureSettings();
    d->viewOverExpoAction->setChecked(on);
    setOverExposureToolTip(on);
}

void EditorWindow::setOverExposureToolTip(bool on)
{
    d->overExposureIndicator->setToolTip(
        on ? i18n("Over-Exposure indicator is enabled")
           : i18n("Over-Exposure indicator is disabled"));
}

void EditorWindow::slotToggleSlideShow()
{
    SlideShowSettings settings;
    settings.readFromConfig();
    slideShow(settings);
}

void EditorWindow::slotSelectionChanged(const QRect& sel)
{
    slotSelectionSetText(sel);
    emit signalSelectionChanged(sel);
}

void EditorWindow::slotSelectionSetText(const QRect& sel)
{
    setToolInfoMessage(QString::fromLatin1("(%1, %2) (%3 x %4)").arg(sel.x()).arg(sel.y()).arg(sel.width()).arg(sel.height()));
}

void EditorWindow::slotComponentsInfo()
{
    LibsInfoDlg* const dlg = new LibsInfoDlg(this);
    dlg->show();
}

void EditorWindow::setToolStartProgress(const QString& toolName)
{
    m_animLogo->start();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, QString::fromUtf8("%1: ").arg(toolName));
}

void EditorWindow::setToolProgress(int progress)
{
    m_nameLabel->setProgressValue(progress);
}

void EditorWindow::setToolStopProgress()
{
    m_animLogo->stop();
    m_nameLabel->setProgressValue(0);
    m_nameLabel->progressBarMode(StatusProgressBar::TextMode);
    slotUpdateItemInfo();
}

void EditorWindow::slotCloseTool()
{
    if (d->toolIface)
    {
        d->toolIface->slotCloseTool();
    }
}

void EditorWindow::slotApplyTool()
{
    if (d->toolIface)
    {
        d->toolIface->slotApplyTool();
    }
}

void EditorWindow::setPreviewModeMask(int mask)
{
    d->previewToolBar->setPreviewModeMask(mask);
}

PreviewToolBar::PreviewMode EditorWindow::previewMode() const
{
    return d->previewToolBar->previewMode();
}

void EditorWindow::setToolInfoMessage(const QString& txt)
{
    d->infoLabel->setAdjustedText(txt);
}

VersionManager* EditorWindow::versionManager() const
{
    return &d->defaultVersionManager;
}

DCategorizedView* EditorWindow::createToolSelectionView()
{
    if (d->selectToolsActionView)
    {
        return d->selectToolsActionView;
    }

    // Create action model
    ActionItemModel* const actionModel = new ActionItemModel(this);
    actionModel->setMode(ActionItemModel::ToplevelMenuCategory | ActionItemModel::SortCategoriesByInsertionOrder);
    QString basicTransformCategory     = i18nc("@title Image transformations", "Basic Transformations");

    // builtin actions
    actionModel->addAction(d->rotateLeftAction, basicTransformCategory);
    actionModel->addAction(d->rotateRightAction, basicTransformCategory);
    actionModel->addAction(d->flipHorizAction, basicTransformCategory);
    actionModel->addAction(d->flipVertAction, basicTransformCategory);
    actionModel->addAction(d->cropAction, basicTransformCategory);
    actionModel->addAction(d->autoCropAction, basicTransformCategory);

    // parse menus for image plugin actions
    actionModel->addActions(menuBar(), d->imagepluginsActionCollection->actions());

    // setup categorized view
    DCategorizedSortFilterProxyModel* const filterModel = actionModel->createFilterModel();

    d->selectToolsActionView = new ActionCategorizedView;
    d->selectToolsActionView->setupIconMode();
    d->selectToolsActionView->setModel(filterModel);
    d->selectToolsActionView->adjustGridSize();

    connect(d->selectToolsActionView, SIGNAL(clicked(QModelIndex)),
            actionModel, SLOT(trigger(QModelIndex)));

    return d->selectToolsActionView;
}

void EditorWindow::setupSelectToolsAction()
{
    QWidgetAction* const viewAction = new QWidgetAction(this);
    viewAction->setDefaultWidget(createToolSelectionView());
    d->selectToolsActionView->setMinimumSize(QSize(400, 400));
    m_selectToolsAction->addAction(viewAction);

    connect(m_selectToolsAction, SIGNAL(aboutToShow()),
            this, SLOT(slotSelectToolsMenuAboutToShow()));

    connect(d->selectToolsActionView, SIGNAL(clicked(QModelIndex)),
            m_selectToolsAction, SLOT(close()));
}

void EditorWindow::slotSelectToolsMenuAboutToShow()
{
    // adjust to window size
    QSize s = size();
    s      /= 2;
    d->selectToolsActionView->setMinimumSize(s);
}

void EditorWindow::slotThemeChanged()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    if (!group.readEntry(d->configUseThemeBackgroundColorEntry, true))
    {
        m_bgColor = group.readEntry(d->configBackgroundColorEntry, QColor(Qt::black));
    }
    else
    {
        m_bgColor = palette().color(QPalette::Base);
    }

    m_canvas->setBackgroundBrush(QBrush(m_bgColor));
    d->toolIface->themeChanged();
}

void EditorWindow::addAction2ContextMenu(const QString& actionName, bool addDisabled)
{
    if (!m_contextMenu)
    {
        return;
    }

    QAction* const action = actionCollection()->action(actionName);

    if (action && (action->isEnabled() || addDisabled))
    {
        m_contextMenu->addAction(action);
    }
}

void EditorWindow::showSideBars(bool visible)
{
    if (visible)
    {
        rightSideBar()->restore(QList<QWidget*>() << thumbBar(), d->fullscreenSizeBackup);
    }
    else
    {
        // See bug #166472, a simple backup()/restore() will hide non-sidebar splitter child widgets
        // in horizontal mode thumbbar wont be member of the splitter, it is just ignored then
        rightSideBar()->backup(QList<QWidget*>() << thumbBar(), &d->fullscreenSizeBackup);
    }
}

void EditorWindow::slotToggleRightSideBar()
{
    rightSideBar()->isExpanded() ? rightSideBar()->shrink()
                                 : rightSideBar()->expand();
}

void EditorWindow::slotPreviousRightSideBarTab()
{
    rightSideBar()->activePreviousTab();
}

void EditorWindow::slotNextRightSideBarTab()
{
    rightSideBar()->activeNextTab();
}

void EditorWindow::showThumbBar(bool visible)
{
    visible ? thumbBar()->restoreVisibility()
            : thumbBar()->hide();
}

bool EditorWindow::thumbbarVisibility() const
{
    return thumbBar()->isVisible();
}

void EditorWindow::customizedFullScreenMode(bool set)
{
    set ? m_canvas->setBackgroundBrush(QBrush(Qt::black))
        : m_canvas->setBackgroundBrush(QBrush(m_bgColor));

    statusBarMenuAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    d->showMenuBarAction->setEnabled(!set);
    m_showBarAction->setEnabled(!set);
}

void EditorWindow::addServicesMenuForUrl(const QUrl& url)
{
    KService::List offers = FileOperation::servicesForOpenWith(QList<QUrl>() << url);

    qCDebug(DIGIKAM_GENERAL_LOG) << offers.count() << " services found to open " << url;

    if (m_servicesMenu)
    {
        delete m_servicesMenu;
        m_servicesMenu = 0;
    }

    if (m_serviceAction)
    {
        delete m_serviceAction;
        m_serviceAction = 0;
    }

    if (!offers.isEmpty())
    {
        m_servicesMenu = new QMenu(this);

        QAction* const serviceAction = m_servicesMenu->menuAction();
        serviceAction->setText(i18n("Open With"));

        foreach(const KService::Ptr& service, offers)
        {
            QString name          = service->name().replace(QLatin1Char('&'), QLatin1String("&&"));
            QAction* const action = m_servicesMenu->addAction(name);
            action->setIcon(QIcon::fromTheme(service->icon()));
            action->setData(service->name());
            d->servicesMap[name]  = service;
        }

        m_servicesMenu->addSeparator();
        m_servicesMenu->addAction(i18n("Other..."));

        m_contextMenu->addAction(serviceAction);

        connect(m_servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }
    else
    {
        m_serviceAction = new QAction(i18n("Open With..."), this);
        m_contextMenu->addAction(m_serviceAction);

        connect(m_servicesMenu, SIGNAL(triggered()),
                this, SLOT(slotOpenWith()));
    }
}

void EditorWindow::openWith(const QUrl& url, QAction* action)
{
    KService::Ptr service;
    QString name = action ? action->data().toString() : QString();

    if (name.isEmpty())
    {
        QPointer<KOpenWithDialog> dlg = new KOpenWithDialog(QList<QUrl>() << url);

        if (dlg->exec() != KOpenWithDialog::Accepted)
        {
            delete dlg;
            return;
        }

        service = dlg->service();

        if (!service)
        {
            // User entered a custom command
            if (!dlg->text().isEmpty())
            {
                KRun::run(dlg->text(), QList<QUrl>() << url, this);
            }

            delete dlg;
            return;
        }

        delete dlg;
    }
    else
    {
        service = d->servicesMap[name];
    }

    KRun::run(*service, QList<QUrl>() << url, this);
}

}  // namespace Digikam
