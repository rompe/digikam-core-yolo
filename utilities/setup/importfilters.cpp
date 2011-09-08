/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Import filters configuration dialog
 *
 * Copyright (C) 2010 by Petri Damstén <petri.damsten@iki.fi>
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

#include "importfilters.moc"

// KDE includes

#include <KComboBox>
#include <KLocale>
#include <KDebug>
#include <KSqueezedTextLabel>
#include <KMimeTypeChooserDialog>

// Qt includes

#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QPainter>

// Local includes

#include "importfilters.h"
#include "filtercombo.h"

namespace Digikam
{

class ImportFilters::ImportFiltersPriv
{
public:

    ImportFiltersPriv()
    {
    }

    ~ImportFiltersPriv()
    {
    }

    QLineEdit*          filterName;
    QCheckBox*          mimeCheckBox;
    KSqueezedTextLabel* mimeLabel;
    QToolButton*        mimeButton;
    QCheckBox*          fileNameCheckBox;
    QLineEdit*          fileNameEdit;
    QCheckBox*          pathCheckBox;
    QLineEdit*          pathEdit;
    QCheckBox*          newFilesCheckBox;
};

// ----------------------------------------------------------------------------------------

ImportFilters::ImportFilters(QWidget* parent) 
    : KDialog(parent), d(new ImportFiltersPriv)
{
    setButtons(KDialog::Cancel | KDialog::Ok);
    setDefaultButton(KDialog::Ok);
    resize(QSize(400, 200));
    setWindowTitle(i18n("Edit Import Filters"));

    QVBoxLayout* verticalLayout   = new QVBoxLayout(this->mainWidget());
    QHBoxLayout* horizontalLayout = 0;
    QSpacerItem* spacer           = 0;
    QLabel* label                 = 0;

    label            = new QLabel(this);
    label->setText(i18n("Name:"));
    verticalLayout->addWidget(label);
    d->filterName    = new QLineEdit(this);
    verticalLayout->addWidget(d->filterName);

    d->mimeCheckBox  = new QCheckBox(this);
    d->mimeCheckBox->setText(i18n("Mime filter:"));
    verticalLayout->addWidget(d->mimeCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->mimeLabel     = new KSqueezedTextLabel(this);
    horizontalLayout->addWidget(d->mimeLabel);
    d->mimeButton    = new QToolButton(this);
    d->mimeButton->setText("...");
    horizontalLayout->addWidget(d->mimeButton);
    verticalLayout->addLayout(horizontalLayout);

    d->fileNameCheckBox = new QCheckBox(this);
    d->fileNameCheckBox->setText(i18n("File name filter:"));
    verticalLayout->addWidget(d->fileNameCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->fileNameEdit  = new QLineEdit(this);
    horizontalLayout->addWidget(d->fileNameEdit);
    verticalLayout->addLayout(horizontalLayout);

    d->pathCheckBox  = new QCheckBox(this);
    d->pathCheckBox->setText(i18n("Path filter:"));
    verticalLayout->addWidget(d->pathCheckBox);
    horizontalLayout = new QHBoxLayout();
    spacer           = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);
    d->pathEdit      = new QLineEdit(this);
    horizontalLayout->addWidget(d->pathEdit);
    verticalLayout->addLayout(horizontalLayout);

    d->newFilesCheckBox = new QCheckBox(this);
    d->newFilesCheckBox->setText(i18n("Show only new files"));
    verticalLayout->addWidget(d->newFilesCheckBox);
    spacer           = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(spacer);

    connect(d->mimeCheckBox, SIGNAL(clicked(bool)), 
            d->mimeButton, SLOT(setEnabled(bool)));

    connect(d->mimeButton, SIGNAL(clicked(bool)),
            this, SLOT(mimeButtonClicked()));

    connect(d->fileNameCheckBox, SIGNAL(clicked(bool)),
            d->fileNameEdit, SLOT(setEnabled(bool)));

    connect(d->pathCheckBox, SIGNAL(clicked(bool)),
            d->pathEdit, SLOT(setEnabled(bool)));

    connect(d->mimeCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(mimeCheckBoxClicked()));

    connect(d->fileNameCheckBox, SIGNAL(clicked()),
            this, SLOT(fileNameCheckBoxClicked()));

    connect(d->pathCheckBox, SIGNAL(clicked()),
            this, SLOT(pathCheckBoxClicked()));
}

ImportFilters::~ImportFilters() 
{
}

void ImportFilters::fileNameCheckBoxClicked()
{
    if (!d->fileNameCheckBox->isChecked())
    {
        d->fileNameEdit->clear();
    }
}

void ImportFilters::pathCheckBoxClicked()
{
    if (!d->pathCheckBox->isChecked())
    {
        d->pathEdit->clear();
    }
}

void ImportFilters::mimeCheckBoxClicked()
{
    if (!d->mimeCheckBox->isChecked())
    {
        d->mimeLabel->clear();
    }
}

void ImportFilters::mimeButtonClicked()
{
    QString text     = i18n("Select the MimeTypes you want for this filter.");
    QStringList list = d->mimeLabel->text().split(';', QString::SkipEmptyParts);
    KMimeTypeChooserDialog dlg(i18n("Select Mime Types"), text, list, "image", this);

    if (dlg.exec() == KDialog::Accepted)
    {
        d->mimeLabel->setText(dlg.chooser()->mimeTypes().join(";"));
    }
}

void ImportFilters::setData(const Filter& filter)
{
    d->filterName->setText(filter.name);
    d->mimeCheckBox->setChecked(!filter.mimeFilter.isEmpty());
    d->mimeLabel->setText(filter.mimeFilter);
    d->mimeButton->setEnabled(!filter.mimeFilter.isEmpty());
    d->fileNameCheckBox->setChecked(!filter.fileFilter.isEmpty());
    d->fileNameEdit->setText(filter.fileFilter.join(";"));
    d->fileNameEdit->setEnabled(!filter.fileFilter.isEmpty());
    d->pathCheckBox->setChecked(!filter.pathFilter.isEmpty());
    d->pathEdit->setText(filter.pathFilter.join(";"));
    d->pathEdit->setEnabled(!filter.pathFilter.isEmpty());
    d->newFilesCheckBox->setChecked(filter.onlyNew);
}

void ImportFilters::getData(Filter* filter)
{
    filter->name       = d->filterName->text();
    filter->mimeFilter = d->mimeLabel->text();
    filter->fileFilter = d->fileNameEdit->text().split(';', QString::SkipEmptyParts);
    filter->pathFilter = d->pathEdit->text().split(';', QString::SkipEmptyParts);
    filter->onlyNew    = d->newFilesCheckBox->isChecked();
}

}  // namespace Digikam