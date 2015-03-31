/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creatingtags
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 1997      by Sven Radej (sven.radej@iname.com)
 * Copyright (c) 1999      by Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * Copyright (c) 1999      by Preston Brown <pbrown@kde.org>
 * Copyright (c) 2000-2001 by Dawit Alemayehu <adawit@kde.org>
 * Copyright (c) 2000-2001 by Carsten Pfeiffer <pfeiffer@kde.org>
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

#include "addtagslineedit.h"

// Qt includes

#include <QKeyEvent>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "addtagscompletionbox.h"
#include "tageditdlg.h"
#include "album.h"
#include "albummodel.h"
#include "albumtreeview.h"
#include "tagscache.h"
#include "databaseaccess.h"
#include "albumdb.h"

namespace Digikam
{

class AddTagsLineEdit::Private
{
public:

    Private()
    {
        completion         = 0;
        tagView            = 0;
        resetFromCompleter = false;
    }

    TagModelCompletion*   completion;
    TagTreeView*          tagView;
    TaggingAction         currentTaggingAction;
    bool                  resetFromCompleter;
    TAlbum*               parentTag;
    TAlbum*               currentTag;
    TagModel*             tagModel;

/*
public:

    TaggingAction makeTaggingAction(const QString& userText);
*/
};

// ---------------------------------------------------------------------------------------

AddTagsLineEdit::AddTagsLineEdit(QWidget* const parent)
    : QLineEdit(parent),
      d(new Private)
{
    d->completion = new TagModelCompletion;
    setCompleter(d->completion);
}

AddTagsLineEdit::~AddTagsLineEdit()
{
    delete d;
}

void AddTagsLineEdit::setModel(TagModel* model)
{
    d->completion->setModel(model);
    d->tagModel = model;
}

void AddTagsLineEdit::setModel(AlbumFilterModel* model)
{
    d->completion->setModel(model);
}

void AddTagsLineEdit::setModel(TagModel* model, TagPropertiesFilterModel* filteredModel, AlbumFilterModel* filterModel)
{
    if (filterModel)
    {
        setModel(filterModel);
    }
    else if (filteredModel)
    {
        setModel(filteredModel);
    }
    else
    {
        setModel(model);
    }
}

void AddTagsLineEdit::setTagTreeView(TagTreeView* view)
{
    if (d->tagView)
    {
        disconnect(d->tagView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                   this, SLOT(setParentTag(QModelIndex)));
    }

    d->tagView = view;
    setParentTag(d->tagView->currentAlbum());

    if (d->tagView)
    {
        //d->completionBox->setParentTag(d->tagView->currentIndex());
        connect(d->tagView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(setParentTag(QModelIndex)));
    }
}

void AddTagsLineEdit::setCurrentTag(TAlbum *album)
{
    d->parentTag = album;
}

void AddTagsLineEdit::setParentTag(QModelIndex& index)
{
    if(index.isValid())
        d->parentTag = dynamic_cast<TAlbum*>(d->tagModel->albumForIndex(index));
}

void AddTagsLineEdit::setParentTag(TAlbum *album)
{
    if(album != NULL)
        d->parentTag = album;
}

void AddTagsLineEdit::completerActivated(QModelIndex index)
{

    int id = index.data(Qt::UserRole+5).toInt();

    qDebug() << "Completer activated" << index.data() << id;

    if(id == -5)
    {
        QMap<QString, QString> errMap;
        AlbumList al = TagEditDlg::createTAlbum(d->tagView->currentAlbum(),
                                                index.data(Qt::UserRole+4).toString(),
                                                QLatin1String("tag"),
                                                QKeySequence(),errMap);
        if(!al.isEmpty())
        {
            d->tagModel->setCheckState((TAlbum*)(al.first()),Qt::Checked);
        }
    }
    else
    {
        TAlbum* const tAlbum = AlbumManager::instance()->findTAlbum(id);
        d->tagModel->setCheckState(tAlbum,Qt::Checked);
    }

    QLineEdit::clear();
}

void AddTagsLineEdit::setAllowExceedBound(bool value)
{
    Q_UNUSED(value);
    // set maximum size of pop-up widget
}

void AddTagsLineEdit::setCompleter(TagModelCompletion *c)
{
    if (d->completion)
        QObject::disconnect(d->completion, 0, this, 0);

    d->completion = c;

    if (!d->completion)
        return;

    d->completion->setWidget(this);

    connect(d->completion, SIGNAL(activated(QModelIndex)),
            this, SLOT(completerActivated(QModelIndex)));
}

void AddTagsLineEdit::focusInEvent(QFocusEvent* f)
{
    QLineEdit::focusInEvent(f);

    // NOTE: Need to disconnect completer from QLineEdit, otherwise
    // we won't be able to clear completion after tag was added

    disconnect(d->completion, SIGNAL(activated(QString)),
               this, SLOT(setText(QString)));
}

void AddTagsLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (d->completion && d->completion->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return; // Let the completer do default behavior
        }
    }

    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;

    if (!isShortcut)
        QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (!d->completion)
        return;

    bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);

    if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
    {
        d->completion->popup()->hide();
        return;
    }

    d->completion->update(text());
    d->completion->popup()->setCurrentIndex(d->completion->completionModel()->index(0, 0));
}

} // namespace Digikam
