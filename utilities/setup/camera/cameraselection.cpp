/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : Camera type selection dialog
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cameraselection.h"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTreeWidget>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QLineEdit>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Libkdcraw includes

#include <KDCRAW/RWidgetUtils>

// Local includes

#include "digikam_config.h"
#include "gpcamera.h"
#include "dxmlguiwindow.h"

using namespace KDcrawIface;

namespace Digikam
{

class CameraSelection::Private
{
public:

    Private() :
        buttons(0),
        portButtonGroup(0),
        usbButton(0),
        serialButton(0),
        portPathLabel(0),
        portPathComboBox(0),
        listView(0),
        titleEdit(0),
        umsMountURL(0),
        searchBar(0)
    {
    }

    QDialogButtonBox* buttons;

    QButtonGroup*     portButtonGroup;

    QRadioButton*     usbButton;
    QRadioButton*     serialButton;

    QLabel*           portPathLabel;

    QComboBox*        portPathComboBox;

    QString           UMSCameraNameActual;
    QString           UMSCameraNameShown;
    QString           PTPCameraNameShown;

    QStringList       serialPortList;

    QTreeWidget*      listView;

    QLineEdit*        titleEdit;

    RFileSelector*    umsMountURL;

    SearchTextBar*    searchBar;
};

CameraSelection::CameraSelection(QWidget* const parent)
    : QDialog(parent), d(new Private)
{
    qApp->setOverrideCursor(Qt::WaitCursor);

    setWindowTitle(i18n("Camera Configuration"));
    setModal(true);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    d->UMSCameraNameActual = QLatin1String("Directory Browse");   // Don't be i18n!
    d->UMSCameraNameShown  = i18n("Mounted Camera");
    d->PTPCameraNameShown  = QLatin1String("USB PTP Class Camera");

    QWidget* const page        = new QWidget(this);
    QGridLayout* mainBoxLayout = new QGridLayout(page);

    // --------------------------------------------------------------

    d->listView = new QTreeWidget(page);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setMinimumWidth(350);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(1);
    d->listView->setHeaderLabels(QStringList() << i18n("Camera List"));
    d->listView->setWhatsThis(i18n("<p>Select the camera name that you want to use here. All "
                                   "default settings on the right panel "
                                   "will be set automatically.</p><p>This list has been generated "
                                   "using the gphoto2 library installed on your computer.</p>"));

    d->searchBar = new SearchTextBar(page, QLatin1String("CameraSelectionSearchBar"));

    // --------------------------------------------------------------

    QGroupBox* const titleBox   = new QGroupBox(i18n("Camera Title"), page);
    QVBoxLayout* const gLayout1 = new QVBoxLayout(titleBox);
    d->titleEdit                = new QLineEdit(titleBox);
    d->titleEdit->setWhatsThis(i18n("<p>Set here the name used in digiKam interface to "
                                    "identify this camera.</p>"));

    gLayout1->addWidget(d->titleEdit);
    gLayout1->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout1->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------------

    QGroupBox* const portBox    = new QGroupBox(i18n("Camera Port Type"), page);
    QVBoxLayout* const gLayout2 = new QVBoxLayout(portBox);
    d->portButtonGroup          = new QButtonGroup(portBox);
    d->portButtonGroup->setExclusive(true);

    d->usbButton = new QRadioButton(i18n("USB"), portBox);
    d->usbButton->setWhatsThis(i18n("<p>Select this option if your camera is connected to your "
                                    "computer using a USB cable.</p>"));

    d->serialButton = new QRadioButton(i18n("Serial"), portBox);
    d->serialButton->setWhatsThis(i18n("<p>Select this option if your camera is connected to your "
                                       "computer using a serial cable.</p>"));

    d->portButtonGroup->addButton(d->usbButton);
    d->portButtonGroup->addButton(d->serialButton);

    gLayout2->addWidget(d->usbButton);
    gLayout2->addWidget(d->serialButton);
    gLayout2->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout2->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------------

    QGroupBox* const portPathBox = new QGroupBox(i18n("Camera Port Path"), page);
    QVBoxLayout* const gLayout3  = new QVBoxLayout(portPathBox);

    d->portPathLabel = new QLabel(portPathBox);
    d->portPathLabel->setText(i18n("Note: only for serial port cameras."));

    d->portPathComboBox = new QComboBox(portPathBox);
    d->portPathComboBox->setDuplicatesEnabled(false);
    d->portPathComboBox->setWhatsThis(i18n("<p>Select the serial port to use on your computer here. "
                                           "This option is only required if you use a serial camera.</p>"));

    gLayout3->addWidget(d->portPathLabel);
    gLayout3->addWidget(d->portPathComboBox);
    gLayout3->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout3->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------------

    QGroupBox* const umsMountBox = new QGroupBox(i18n("Camera Mount Path"), page);
    QVBoxLayout* const gLayout4  = new QVBoxLayout(umsMountBox);

    QLabel* const umsMountLabel = new QLabel(umsMountBox);
    umsMountLabel->setText(i18n("Note: only for USB/IEEE mass storage cameras."));

    d->umsMountURL = new RFileSelector(umsMountBox);
    d->umsMountURL->lineEdit()->setText(QLatin1String("/mnt/camera"));
    d->umsMountURL->fileDialog()->setFileMode(QFileDialog::Directory);
    d->umsMountURL->setWhatsThis(i18n("<p>Set here the mount path to use on your computer. This "
                                      "option is only required if you use a <b>USB Mass Storage</b> "
                                      "camera.</p>"));

    gLayout4->addWidget(umsMountLabel);
    gLayout4->addWidget(d->umsMountURL);
    gLayout4->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout4->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------------

    QWidget* const box2   = new QWidget(page);
    QGridLayout* gLayout5 = new QGridLayout(box2);

    QLabel* const logo = new QLabel(box2);
    logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-digikam.png")))
                    .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* const link = new QLabel(box2);
    link->setText(i18n("<p>To set a <b>USB Mass Storage</b> camera<br/>"
                       "(which looks like a removable drive when mounted<br/>"
                       "on your desktop), please use<br/>"
                       "<a href=\"umscamera\">%1</a> from the camera list.</p>",
                       d->UMSCameraNameShown));

    QLabel* const link2 = new QLabel(box2);
    link2->setText(i18n("<p>To set a <b>Generic PTP USB Device</b><br/>"
                        "(which uses Picture Transfer Protocol), please<br/>"
                        "use <a href=\"ptpcamera\">%1</a> from the camera list.</p>",
                        d->PTPCameraNameShown));

    QLabel* const explanation = new QLabel(box2);
    explanation->setOpenExternalLinks(true);
    explanation->setText(i18n("<p>A complete list of camera settings to use is<br/>"
                              "available at <a href='http://www.teaser.fr/~hfiguiere/linux/digicam.html'>"
                              "this URL</a>.</p>"));

    gLayout5->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout5->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    gLayout5->addWidget(logo,        0, 0, 1, 1);
    gLayout5->addWidget(link,        0, 1, 2, 1);
    gLayout5->addWidget(link2,       2, 1, 2, 1);
    gLayout5->addWidget(explanation, 4, 1, 2, 1);

    // --------------------------------------------------------------

    mainBoxLayout->addWidget(d->listView,  0, 0, 6, 1);
    mainBoxLayout->addWidget(d->searchBar, 7, 0, 1, 1);
    mainBoxLayout->addWidget(titleBox,     0, 1, 1, 1);
    mainBoxLayout->addWidget(portBox,      1, 1, 1, 1);
    mainBoxLayout->addWidget(portPathBox,  2, 1, 1, 1);
    mainBoxLayout->addWidget(umsMountBox,  3, 1, 1, 1);
    mainBoxLayout->addWidget(box2,         4, 1, 2, 1);
    mainBoxLayout->setColumnStretch(0, 10);
    mainBoxLayout->setRowStretch(6, 10);
    mainBoxLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    mainBoxLayout->setMargin(0);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // Connections --------------------------------------------------

    connect(link, SIGNAL(linkActivated(QString)),
            this, SLOT(slotUMSCameraLinkUsed()));

    connect(link2, SIGNAL(linkActivated(QString)),
            this, SLOT(slotPTPCameraLinkUsed()));

    connect(d->listView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem*,int)));

    connect(d->portButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotPortChanged()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOkClicked()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // Initialize  --------------------------------------------------

#ifndef HAVE_GPHOTO2
    // If digiKam is compiled without Gphoto2 support, we hide widgets relevant.
    d->listView->hide();
    d->searchBar->hide();
    box2->hide();
    slotUMSCameraLinkUsed();
#else
    getCameraList();
    getSerialPortList();
#endif /* HAVE_GPHOTO2 */

    qApp->restoreOverrideCursor();
}

CameraSelection::~CameraSelection()
{
    delete d;
}

void CameraSelection::slotUMSCameraLinkUsed()
{
    QList<QTreeWidgetItem*> list = d->listView->findItems(d->UMSCameraNameShown, Qt::MatchExactly, 0);

    if (!list.isEmpty())
    {
        QTreeWidgetItem* const item = list.first();

        if (item)
        {
            d->listView->setCurrentItem(item);
            d->listView->scrollToItem(item);
        }
    }
}

void CameraSelection::slotPTPCameraLinkUsed()
{
    QList<QTreeWidgetItem*> list = d->listView->findItems(d->PTPCameraNameShown, Qt::MatchExactly, 0);

    if (!list.isEmpty())
    {
        QTreeWidgetItem* const item = list.first();

        if (item)
        {
            d->listView->setCurrentItem(item);
            d->listView->scrollToItem(item);
        }
    }
}

void CameraSelection::setCamera(const QString& title, const QString& model,
                                const QString& port,  const QString& path)
{
    QString camModel(model);

    if (camModel == d->UMSCameraNameActual)
    {
        camModel = d->UMSCameraNameShown;
    }

    QList<QTreeWidgetItem*> list = d->listView->findItems(camModel, Qt::MatchExactly, 0);

    if (!list.isEmpty())
    {
        QTreeWidgetItem* const item = list.first();

        if (!item)
        {
            return;
        }

        d->listView->setCurrentItem(item);
        d->listView->scrollToItem(item);

        d->titleEdit->setText(title);

        if (port.contains(QLatin1String("usb")))
        {
            d->usbButton->setChecked(true);
            slotPortChanged();
        }
        else if (port.contains(QLatin1String("serial")))
        {
            d->serialButton->setChecked(true);

            for (int i = 0 ; i < d->portPathComboBox->count() ; ++i)
            {
                if (port == d->portPathComboBox->itemText(i))
                {
                    d->portPathComboBox->setCurrentIndex(i);
                    break;
                }
            }

            slotPortChanged();
        }

        d->umsMountURL->lineEdit()->setText(path);
    }
}

void CameraSelection::getCameraList()
{
    int count = 0;
    QStringList clist;
    QString cname;

    GPCamera::getSupportedCameras(count, clist);

    for (int i = 0 ; i < count ; ++i)
    {
        cname = clist.at(i);

        if (cname == d->UMSCameraNameActual)
        {
            new QTreeWidgetItem(d->listView, QStringList() << d->UMSCameraNameShown);
        }
        else
        {
            new QTreeWidgetItem(d->listView, QStringList() << cname);
        }
    }
}

void CameraSelection::getSerialPortList()
{
    QStringList plist;

    GPCamera::getSupportedPorts(plist);

    d->serialPortList.clear();

    for (int i = 0; i < plist.count() ; ++i)
    {
        if ((plist.at(i)).startsWith(QLatin1String("serial:")))
        {
            d->serialPortList.append(plist.at(i));
        }
    }
}

void CameraSelection::slotSelectionChanged(QTreeWidgetItem* item, int)
{
    if (!item)
    {
        return;
    }

    QString model(item->text(0));

    if (model == d->UMSCameraNameShown)
    {
        model = d->UMSCameraNameActual;

        d->titleEdit->setText(model);

        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(false);
        d->serialButton->setEnabled(false);
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(false);
        d->usbButton->setEnabled(false);
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->insertItem(0, QLatin1String("NONE"));
        d->portPathComboBox->setEnabled(false);

        d->umsMountURL->setEnabled(true);
        d->umsMountURL->lineEdit()->clear();
        d->umsMountURL->lineEdit()->setText(QLatin1String("/mnt/camera"));
        return;
    }
    else
    {
        d->umsMountURL->setEnabled(true);
        d->umsMountURL->lineEdit()->clear();
        d->umsMountURL->lineEdit()->setText(QLatin1String("/"));
        d->umsMountURL->setEnabled(false);
    }

    d->titleEdit->setText(model);

    QStringList plist;
    GPCamera::getCameraSupportedPorts(model, plist);

    if (plist.contains(QLatin1String("serial")))
    {
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(true);
    }
    else
    {
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(false);
        d->serialButton->setEnabled(false);
    }

    if (plist.contains(QLatin1String("usb")))
    {
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(true);
    }
    else
    {
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(false);
        d->usbButton->setEnabled(false);
    }

    slotPortChanged();
}

void CameraSelection::slotPortChanged()
{
    if (d->usbButton->isChecked())
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItem(0, QLatin1String("usb:"));
        d->portPathComboBox->setEnabled(false);
        return;
    }

    if (d->serialButton->isChecked())
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItems(0, d->serialPortList);
    }
}

QString CameraSelection::currentTitle() const
{
    return d->titleEdit->text();
}

QString CameraSelection::currentModel() const
{
    QTreeWidgetItem* const item = d->listView->currentItem();

    if (!item)
    {
        return QString();
    }

    QString model(item->text(0));

    if (model == d->UMSCameraNameShown)
    {
        model = d->UMSCameraNameActual;
    }

    return model;
}

QString CameraSelection::currentPortPath() const
{
    return d->portPathComboBox->currentText();
}

QString CameraSelection::currentCameraPath() const
{
    return d->umsMountURL->lineEdit()->text();
}

void CameraSelection::slotOkClicked()
{
    emit signalOkClicked(currentTitle(),    currentModel(),
                         currentPortPath(), currentCameraPath());
    accept();
}

void CameraSelection::slotSearchTextChanged(const SearchTextSettings& settings)
{
    bool query     = false;
    QString search = settings.text;

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        QTreeWidgetItem* const item  = *it;

        if (item->text(0).contains(search, settings.caseSensitive))
        {
            query = true;
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }

        ++it;
    }

    d->searchBar->slotSearchResult(query);
}

void CameraSelection::slotHelp()
{
    DXmlGuiWindow::openHandbook(QLatin1String("cameraselection.anchor"), QLatin1String("digikam"));
}

}  // namespace Digikam
