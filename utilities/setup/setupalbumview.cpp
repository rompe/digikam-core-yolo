/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-01
 * Description : album view configuration setup tab
 *
 * Copyright (C) 2003-2004 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupalbumview.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <khistorycombobox.h>

// Local includes

#include "digikam_debug.h"
#include "thumbnailsize.h"
#include "applicationsettings.h"
#include "dfontselect.h"
#include "fullscreensettings.h"
#include "dxmlguiwindow.h"
#include "previewsettings.h"

namespace Digikam
{

class SetupAlbumView::Private
{
public:

    Private() :
        useLargeThumbsOriginal(false),
        useLargeThumbsShowedInfo(false),
        iconTreeThumbLabel(0),
        iconShowNameBox(0),
        iconShowSizeBox(0),
        iconShowDateBox(0),
        iconShowModDateBox(0),
        iconShowResolutionBox(0),
        iconShowAspectRatioBox(0),
        iconShowTitleBox(0),
        iconShowCommentsBox(0),
        iconShowTagsBox(0),
        iconShowOverlaysBox(0),
        iconShowRatingBox(0),
        iconShowFormatBox(0),
        iconShowCoordinatesBox(0),
        previewFastPreview(0),
        previewFullView(0),
        previewRawMode(0),
        previewShowIcons(0),
        showFolderTreeViewItemsCount(0),
        largeThumbsBox(0),
        iconTreeThumbSize(0),
        leftClickActionComboBox(0),
        iconViewFontSelect(0),
        treeViewFontSelect(0),
        fullScreenSettings(0),
        groupRegexGB(0),
        groupRegexRawJpeg(0),
        groupRegexCommonModifiers(0),
        groupRegexCustom(0),
        groupRegexCustomHistoryBox(0)
    {
    }

    bool                useLargeThumbsOriginal;
    bool                useLargeThumbsShowedInfo;

    QLabel*             iconTreeThumbLabel;

    QCheckBox*          iconShowNameBox;
    QCheckBox*          iconShowSizeBox;
    QCheckBox*          iconShowDateBox;
    QCheckBox*          iconShowModDateBox;
    QCheckBox*          iconShowResolutionBox;
    QCheckBox*          iconShowAspectRatioBox;
    QCheckBox*          iconShowTitleBox;
    QCheckBox*          iconShowCommentsBox;
    QCheckBox*          iconShowTagsBox;
    QCheckBox*          iconShowOverlaysBox;
    QCheckBox*          iconShowRatingBox;
    QCheckBox*          iconShowFormatBox;
    QCheckBox*          iconShowCoordinatesBox;
    QRadioButton*       previewFastPreview;
    QRadioButton*       previewFullView;
    QComboBox*          previewRawMode;
    QCheckBox*          previewShowIcons;
    QCheckBox*          showFolderTreeViewItemsCount;
    QCheckBox*          largeThumbsBox;

    QComboBox*          iconTreeThumbSize;
    QComboBox*          leftClickActionComboBox;

    DFontSelect*        iconViewFontSelect;
    DFontSelect*        treeViewFontSelect;

    FullScreenSettings* fullScreenSettings;

    QGroupBox*          groupRegexGB;
    QRadioButton*       groupRegexRawJpeg;
    QRadioButton*       groupRegexCommonModifiers;
    QRadioButton*       groupRegexCustom;
    KHistoryComboBox*   groupRegexCustomHistoryBox;
};

SetupAlbumView::SetupAlbumView(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const iconViewGroup = new QGroupBox(i18n("Icon-View Options"), panel);
    QGridLayout* const grid        = new QGridLayout(iconViewGroup);

    d->iconShowNameBox       = new QCheckBox(i18n("Show file&name"), iconViewGroup);
    d->iconShowNameBox->setWhatsThis(i18n("Set this option to show the filename below the image thumbnail."));

    d->iconShowSizeBox       = new QCheckBox(i18n("Show file si&ze"), iconViewGroup);
    d->iconShowSizeBox->setWhatsThis(i18n("Set this option to show the file size below the image thumbnail."));

    d->iconShowDateBox       = new QCheckBox(i18n("Show camera creation &date"), iconViewGroup);
    d->iconShowDateBox->setWhatsThis(i18n("Set this option to show the camera creation date "
                                          "below the image thumbnail."));

    d->iconShowModDateBox    = new QCheckBox(i18n("Show file &modification date (if different than creation date)"), iconViewGroup);
    d->iconShowModDateBox->setWhatsThis(i18n("Set this option to show the file modification date "
                                             "below the image thumbnail if it's different than camera creation date. "
                                             "This option is useful to identify quickly which items have been modified."));

    d->iconShowResolutionBox = new QCheckBox(i18n("Show ima&ge dimensions"), iconViewGroup);
    d->iconShowResolutionBox->setWhatsThis(i18n("Set this option to show the image size in pixels "
                                                "below the image thumbnail."));

    d->iconShowAspectRatioBox = new QCheckBox(i18n("Show image aspect ratio"), iconViewGroup);
    d->iconShowAspectRatioBox->setWhatsThis(i18n("Set this option to show the image aspect ratio "
                                                "below the image thumbnail."));

    d->iconShowFormatBox      = new QCheckBox(i18n("Show image Format"), iconViewGroup);
    d->iconShowFormatBox->setWhatsThis(i18n("Set this option to show image format over image thumbnail."));

    d->iconShowTitleBox       = new QCheckBox(i18n("Show digiKam tit&le"), iconViewGroup);
    d->iconShowTitleBox->setWhatsThis(i18n("Set this option to show the digiKam title "
                                           "below the image thumbnail."));

    d->iconShowCommentsBox    = new QCheckBox(i18n("Show digiKam &captions"), iconViewGroup);
    d->iconShowCommentsBox->setWhatsThis(i18n("Set this option to show the digiKam captions "
                                              "below the image thumbnail."));

    d->iconShowTagsBox        = new QCheckBox(i18n("Show digiKam &tags"), iconViewGroup);
    d->iconShowTagsBox->setWhatsThis(i18n("Set this option to show the digiKam tags "
                                          "below the image thumbnail."));

    d->iconShowRatingBox      = new QCheckBox(i18n("Show digiKam &rating"), iconViewGroup);
    d->iconShowRatingBox->setWhatsThis(i18n("Set this option to show the digiKam rating "
                                            "below the image thumbnail."));

    d->iconShowOverlaysBox    = new QCheckBox(i18n("Show rotation overlay buttons"), iconViewGroup);
    d->iconShowOverlaysBox->setWhatsThis(i18n("Set this option to show overlay buttons on "
                                              "the image thumbnail for image rotation."));

    d->iconShowCoordinatesBox = new QCheckBox(i18n("Show Geolocation Indicator"), iconViewGroup);
    d->iconShowCoordinatesBox->setWhatsThis(i18n("Set this option to indicate if image has geolocation information."));

    QLabel* leftClickLabel     = new QLabel(i18n("Thumbnail click action:"), iconViewGroup);
    d->leftClickActionComboBox = new QComboBox(iconViewGroup);
    d->leftClickActionComboBox->addItem(i18n("Show embedded view"), ApplicationSettings::ShowPreview);
    d->leftClickActionComboBox->addItem(i18n("Start image editor"), ApplicationSettings::StartEditor);
    d->leftClickActionComboBox->setToolTip(i18n("Choose what should happen when you click on a thumbnail."));

    d->iconViewFontSelect = new DFontSelect(i18n("Icon View font:"), panel);
    d->iconViewFontSelect->setToolTip(i18n("Select here the font used to display text in Icon Views."));

    d->largeThumbsBox = new QCheckBox(i18n("Use large thumbnail size for high screen resolution"), iconViewGroup);
    d->largeThumbsBox->setWhatsThis(i18n("Set this option to render icon-view with large thumbnail size, for example in case of 4K monitor is used.\n"
                                         "By default this option is turned off and the maximum thumbnail size is limited to 256x256 pixels. "
                                         "When this option is enabled, thumbnail size can be extended to 512x512 pixels.\n"
                                         "This option will store more data in thumbnail database and will use more system memory. "
                                         "digiKam needs to be restarted to take effect, and Rebuild Thumbnails option from Maintenance tool "
                                         "needs to be processed over whole collections."));

    grid->addWidget(d->iconShowNameBox,          0, 0, 1, 1);
    grid->addWidget(d->iconShowSizeBox,          1, 0, 1, 1);
    grid->addWidget(d->iconShowDateBox,          2, 0, 1, 1);
    grid->addWidget(d->iconShowModDateBox,       3, 0, 1, 1);
    grid->addWidget(d->iconShowResolutionBox,    4, 0, 1, 1);
    grid->addWidget(d->iconShowAspectRatioBox,   5, 0, 1, 1);
    grid->addWidget(d->iconShowFormatBox,        6, 0, 1, 1);

    grid->addWidget(d->iconShowTitleBox,         0, 1, 1, 1);
    grid->addWidget(d->iconShowCommentsBox,      1, 1, 1, 1);
    grid->addWidget(d->iconShowTagsBox,          2, 1, 1, 1);
    grid->addWidget(d->iconShowRatingBox,        3, 1, 1, 1);
    grid->addWidget(d->iconShowOverlaysBox,      4, 1, 1, 1);
    grid->addWidget(d->iconShowCoordinatesBox,   5, 1, 1, 1);

    grid->addWidget(leftClickLabel,              7, 0, 1, 1);
    grid->addWidget(d->leftClickActionComboBox,  7, 1, 1, 1);
    grid->addWidget(d->iconViewFontSelect,       8, 0, 1, 2);
    grid->addWidget(d->largeThumbsBox,           9, 0, 1, 2);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    QGroupBox* const folderViewGroup = new QGroupBox(i18n("Folder View Options"), panel);
    QGridLayout* const grid2         = new QGridLayout(folderViewGroup);

    d->iconTreeThumbLabel = new QLabel(i18n("Tree View thumbnail size:"), folderViewGroup);
    d->iconTreeThumbSize  = new QComboBox(folderViewGroup);
    d->iconTreeThumbSize->addItem(QLatin1String("16"));
    d->iconTreeThumbSize->addItem(QLatin1String("22"));
    d->iconTreeThumbSize->addItem(QLatin1String("32"));
    d->iconTreeThumbSize->addItem(QLatin1String("48"));
    d->iconTreeThumbSize->setToolTip(i18n("Set this option to configure the size in pixels of "
                                          "the Tree View thumbnails in digiKam's sidebars."));

    d->treeViewFontSelect = new DFontSelect(i18n("Tree View font:"), folderViewGroup);
    d->treeViewFontSelect->setToolTip(i18n("Select here the font used to display text in Tree Views."));

    d->showFolderTreeViewItemsCount = new QCheckBox(i18n("Show a count of items in Tree Views"), folderViewGroup);

    grid2->addWidget(d->iconTreeThumbLabel,           0, 0, 1, 1);
    grid2->addWidget(d->iconTreeThumbSize,            0, 1, 1, 1);
    grid2->addWidget(d->treeViewFontSelect,           1, 0, 1, 2);
    grid2->addWidget(d->showFolderTreeViewItemsCount, 2, 0, 1, 2);
    grid2->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid2->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    QGroupBox* const interfaceOptionsGroup = new QGroupBox(i18n("Preview Options"), panel);
    QGridLayout* const grid3               = new QGridLayout(interfaceOptionsGroup);

    d->previewFastPreview   = new QRadioButton(i18nc("@option:radio",
                                                     "Embedded view shows a small, quick preview"));
    d->previewFullView      = new QRadioButton(i18nc("@option:radio",
                                                     "Embedded view shows the full image"));
    QLabel* const rawPreviewLabel = new QLabel(i18nc("@label:listbox Mode of RAW preview decoding:",
                                               "Raw images:"));
    d->previewRawMode       = new QComboBox;
    d->previewRawMode->addItem(i18nc("@option:inlistbox Automatic choice of RAW image preview source",
                                     "Automatic"), PreviewSettings::RawPreviewAutomatic);
    d->previewRawMode->addItem(i18nc("@option:inlistbox Embedded preview as RAW image preview source",
                                     "Embedded preview"), PreviewSettings::RawPreviewFromEmbeddedPreview);
    d->previewRawMode->addItem(i18nc("@option:inlistbox Original, half-size data as RAW image preview source",
                                     "Raw data in half size"), PreviewSettings::RawPreviewFromRawHalfSize);

    d->previewShowIcons = new QCheckBox(i18n("Show icons and text over preview"), interfaceOptionsGroup);
    d->previewShowIcons->setWhatsThis(i18n("Uncheck this if you do not want to see icons and text in the image preview."));

    grid3->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid3->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid3->addWidget(d->previewFastPreview, 0, 0, 1, 2);
    grid3->addWidget(d->previewFullView,    1, 0, 1, 2);
    grid3->addWidget(rawPreviewLabel,       2, 0, 1, 1);
    grid3->addWidget(d->previewRawMode,     2, 1, 1, 1);
    grid3->addWidget(d->previewShowIcons,   3, 0, 1, 2);

    d->previewFastPreview->setChecked(true);
    d->previewRawMode->setCurrentIndex(0);
    d->previewRawMode->setEnabled(false);

    // --------------------------------------------------------

    d->fullScreenSettings = new FullScreenSettings(FS_ALBUMGUI, panel);

    // --------------------------------------------------------

    QGroupBox* const groupRegexGB = new QGroupBox(i18n("Group by filename"), panel);
    QVBoxLayout* const vlayGroupRegex = new QVBoxLayout(groupRegexGB);

    d->groupRegexRawJpeg = new QRadioButton(i18n("Group RAW and Jpeg"));
    d->groupRegexRawJpeg->setWhatsThis(i18n("<p>Group files with same base name like basename.raw, "
                                            "basename.jpg, basename.tif, etc.</p>"));

    d->groupRegexCommonModifiers = new QRadioButton(i18n("Group files using common modifiers, generated by popular software"));
    d->groupRegexCommonModifiers->setWhatsThis( i18n("<p>Group files with common modifiers in base "
                                        "name, generated by popular software. "
                                        "Example basename.jpg, basename_(01).jpg, "
                                        "basename-Edit.jpg, basename_v1.jpg, basename_shotwell.jpg</p>"));

    d->groupRegexCustom = new QRadioButton(i18n("Custom regular expression string for \"Group selected items by filename\". "));
    d->groupRegexCustom->setWhatsThis( i18n("<p>Perl regular expression to be used for <i>\"Group Group selected items by filename\"</p>"));

    //KHBox* hbox1 = new KHBox(groupRegexGB);
    d->groupRegexCustomHistoryBox = new KHistoryComboBox(groupRegexGB);
    d->groupRegexCustomHistoryBox->setWhatsThis( i18n("<p>Here you can define a perl regular expression to group files by. "
                                     "The expression must feature one group (a parentized part of the expression). "
                                     "All file names matching the same substring into this group will be grouped.</p>"
                                     "<p>Example:</p>"
                                     "<p>If the regular expression is set to <i>\"(^DSC\\d{5})\"</i>, "
                                     "the files <i>DSC00001.jpg</i> and <i>DSC00001edited.jpg</i> are grouped but "
                                     "<i>DSC00002.jpg</i> is not.</p>"));

    QGridLayout* const gridGroupRegex = new QGridLayout;
    gridGroupRegex->addWidget(d->groupRegexCustom, 0, 0, 1, 2);
    gridGroupRegex->addWidget(d->groupRegexCustomHistoryBox, 1, 1);
    gridGroupRegex->setColumnMinimumWidth(0, 10);

    vlayGroupRegex->addWidget(d->groupRegexRawJpeg);
    vlayGroupRegex->addWidget(d->groupRegexCommonModifiers);
    vlayGroupRegex->addLayout(gridGroupRegex);

    // --------------------------------------------------------

    layout->setMargin(0);
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->addWidget(iconViewGroup);
    layout->addWidget(folderViewGroup);
    layout->addWidget(interfaceOptionsGroup);
    layout->addWidget(d->fullScreenSettings);
    layout->addWidget(groupRegexGB);
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();
    adjustSize();

    // --------------------------------------------------------

    connect(d->previewFullView, SIGNAL(toggled(bool)),
            d->previewRawMode, SLOT(setEnabled(bool)));

    connect(d->largeThumbsBox, SIGNAL(toggled(bool)),
            this, SLOT(slotUseLargeThumbsToggled(bool)));

    connect(d->groupRegexRawJpeg, SIGNAL(toggled(bool)),
            this, SLOT(slotGroupRegexToggled()));
    connect(d->groupRegexCommonModifiers, SIGNAL(toggled(bool)),
            this, SLOT(slotGroupRegexToggled()));
    connect(d->groupRegexCustomHistoryBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotGroupRegexToggled()));
    connect(d->groupRegexCustomHistoryBox, SIGNAL(focusOutEvent(*QFocusEvent)),
            this, SLOT(slotGroupRegexToggled()));
    this->slotGroupRegexToggled();

    // --------------------------------------------------------
}

void SetupAlbumView::slotGroupRegexToggled()
{
    QString regexRawJpeg = QString::fromLatin1("(.+)\\.[^.]+(?# detect files with different extensions)");
    QString regexCommonModifiers = QString::fromLatin1("(.+?)( \\(.+\\)|_\\d+|_v\\d+|-Edit|_\\w+_shotwell)?\\.[^.]+(?# detect common modifier)");

    if (d->groupRegexRawJpeg->isChecked())
    {
        d->groupRegexCustomHistoryBox->setCurrentItem(regexRawJpeg, true);
        d->groupRegexCustomHistoryBox->setEnabled(false);
    }
    else if (d->groupRegexCommonModifiers->isChecked())
    {
        d->groupRegexCustomHistoryBox->setCurrentItem(regexCommonModifiers, true);
        d->groupRegexCustomHistoryBox->setEnabled(false);
    }
    else
    {
        if (d->groupRegexCustomHistoryBox->isEnabled())
        {
            d->groupRegexCustom->setChecked(true);
            // Initial state should also be regexRawJpeg.
            if (d->groupRegexCustomHistoryBox->currentText() == regexRawJpeg ||
                 d->groupRegexCustomHistoryBox->currentText() == QString::fromLatin1(""))
            {
                d->groupRegexRawJpeg->setChecked(true);
            }
            else if (d->groupRegexCustomHistoryBox->currentText() == regexCommonModifiers)
            {
                d->groupRegexCommonModifiers->setChecked(true);
            }
        }
        else
        {
            d->groupRegexCustomHistoryBox->setEnabled(true);
        }
    }
}

SetupAlbumView::~SetupAlbumView()
{
    delete d;
}

void SetupAlbumView::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setTreeViewIconSize(d->iconTreeThumbSize->currentText().toInt());
    settings->setTreeViewFont(d->treeViewFontSelect->font());
    settings->setIconShowName(d->iconShowNameBox->isChecked());
    settings->setIconShowTags(d->iconShowTagsBox->isChecked());
    settings->setIconShowSize(d->iconShowSizeBox->isChecked());
    settings->setIconShowDate(d->iconShowDateBox->isChecked());
    settings->setIconShowModDate(d->iconShowModDateBox->isChecked());
    settings->setIconShowResolution(d->iconShowResolutionBox->isChecked());
    settings->setIconShowAspectRatio(d->iconShowAspectRatioBox->isChecked());
    settings->setIconShowTitle(d->iconShowTitleBox->isChecked());
    settings->setIconShowComments(d->iconShowCommentsBox->isChecked());
    settings->setIconShowOverlays(d->iconShowOverlaysBox->isChecked());
    settings->setIconShowCoordinates(d->iconShowCoordinatesBox->isChecked());
    settings->setIconShowRating(d->iconShowRatingBox->isChecked());
    settings->setIconShowImageFormat(d->iconShowFormatBox->isChecked());
    settings->setIconViewFont(d->iconViewFontSelect->font());

    settings->setItemLeftClickAction((ApplicationSettings::ItemLeftClickAction)
                                     d->leftClickActionComboBox->currentIndex());

    PreviewSettings previewSettings;
    previewSettings.quality = d->previewFastPreview->isChecked() ? PreviewSettings::FastPreview : PreviewSettings::HighQualityPreview;
    previewSettings.rawLoading = (PreviewSettings::RawLoading)d->previewRawMode->itemData(d->previewRawMode->currentIndex()).toInt();
    settings->setPreviewSettings(previewSettings);

    settings->setPreviewShowIcons(d->previewShowIcons->isChecked());
    settings->setShowFolderTreeViewItemsCount(d->showFolderTreeViewItemsCount->isChecked());
    settings->setGroupRegexLastUsedPattern(d->groupRegexCustomHistoryBox->currentText());
    settings->setGroupRegexPatternHistoryList(d->groupRegexCustomHistoryBox->historyItems());
    settings->saveSettings();

    KConfigGroup group = KSharedConfig::openConfig()->group(settings->generalConfigGroupName());
    d->fullScreenSettings->saveSettings(group);

    // Method ThumbnailSize::setUseLargeThumbs() is not called here to prevent dysfunction between Thumbs DB and icon if
    // thumb size is over 256 and when large thumbs size support is disabled. digiKam need to be restarted to take effect.
    ThumbnailSize::saveSettings(group, d->largeThumbsBox->isChecked());
}

void SetupAlbumView::readSettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    if (settings->getTreeViewIconSize() == 16)
    {
        d->iconTreeThumbSize->setCurrentIndex(0);
    }
    else if (settings->getTreeViewIconSize() == 22)
    {
        d->iconTreeThumbSize->setCurrentIndex(1);
    }
    else if (settings->getTreeViewIconSize() == 32)
    {
        d->iconTreeThumbSize->setCurrentIndex(2);
    }
    else
    {
        d->iconTreeThumbSize->setCurrentIndex(3);
    }

    d->treeViewFontSelect->setFont(settings->getTreeViewFont());
    d->iconShowNameBox->setChecked(settings->getIconShowName());
    d->iconShowTagsBox->setChecked(settings->getIconShowTags());
    d->iconShowSizeBox->setChecked(settings->getIconShowSize());
    d->iconShowDateBox->setChecked(settings->getIconShowDate());
    d->iconShowModDateBox->setChecked(settings->getIconShowModDate());
    d->iconShowResolutionBox->setChecked(settings->getIconShowResolution());
    d->iconShowAspectRatioBox->setChecked(settings->getIconShowAspectRatio());
    d->iconShowTitleBox->setChecked(settings->getIconShowTitle());
    d->iconShowCommentsBox->setChecked(settings->getIconShowComments());
    d->iconShowOverlaysBox->setChecked(settings->getIconShowOverlays());
    d->iconShowCoordinatesBox->setChecked(settings->getIconShowCoordinates());
    d->iconShowRatingBox->setChecked(settings->getIconShowRating());
    d->iconShowFormatBox->setChecked(settings->getIconShowImageFormat());
    d->iconViewFontSelect->setFont(settings->getIconViewFont());

    d->leftClickActionComboBox->setCurrentIndex((int)settings->getItemLeftClickAction());

    PreviewSettings previewSettings = settings->getPreviewSettings();
    if (previewSettings.quality == PreviewSettings::FastPreview)
    {
        d->previewFastPreview->setChecked(true);
    }
    else
    {
        d->previewFullView->setChecked(true);
    }
    d->previewRawMode->setCurrentIndex(d->previewRawMode->findData(previewSettings.rawLoading));

    d->previewShowIcons->setChecked(settings->getPreviewShowIcons());
    d->showFolderTreeViewItemsCount->setChecked(settings->getShowFolderTreeViewItemsCount());

    d->groupRegexCustomHistoryBox->setHistoryItems(settings->getGroupRegexPatternHistoryList());
    d->groupRegexCustomHistoryBox->setCurrentItem(settings->getGroupRegexLastUsedPattern(),true);

    KConfigGroup group = KSharedConfig::openConfig()->group(settings->generalConfigGroupName());
    d->fullScreenSettings->readSettings(group);

    ThumbnailSize::readSettings(group);
    d->useLargeThumbsOriginal = ThumbnailSize::getUseLargeThumbs();
    d->largeThumbsBox->setChecked(d->useLargeThumbsOriginal);
}

bool SetupAlbumView::useLargeThumbsHasChanged() const
{
    return d->largeThumbsBox->isChecked() != d->useLargeThumbsOriginal;
}

void SetupAlbumView::slotUseLargeThumbsToggled(bool b)
{
    // Show info if large thumbs were enabled, and only once.
    if (b && d->useLargeThumbsShowedInfo && useLargeThumbsHasChanged())
    {
        d->useLargeThumbsShowedInfo = true;
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info",
                                       "This option changes the size in which thumbnails are generated. "
                                       "You need to restart digiKam for this option to take effect. "
                                       "Furthermore, you need to regenerate all already stored thumbnails via "
                                       "the <interface>Tools / Maintenance</interface> menu."));
    }
}

}  // namespace Digikam
