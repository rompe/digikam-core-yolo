/* ============================================================
 * File  : dirselectdialog.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-18
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
 * 
 * Modified from kdirselectdialog
 * Copyright (C) 2001 Michael Jarrett <michaelj@corel.com>
 * Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <klocale.h>
#include <kfiletreeview.h>
#include <kaction.h>
#include <kio/job.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qpopupmenu.h>
#include <qdir.h>
#include <qheader.h>
#include <qlabel.h>
#include <qframe.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

#include "dirselectdialog.h"

DirSelectDialog::DirSelectDialog(const QString& rootDir,
                                 const QString& startDir,
                                 QWidget* parent,
                                 QString header)
    : KDialogBase(parent, 0, true, i18n("Select Album"), User1|Ok|Cancel)
{
    setButtonText(User1, i18n("&New Album"));
    
    QFrame *page     = makeMainWidget();
    QVBoxLayout* lay = new QVBoxLayout(page, 5, 5);

    if (!header.isEmpty())
    {
        QLabel* head = new QLabel(header, page);
        lay->addWidget(head);
        QFrame* hline = new QFrame(page);
        hline->setFrameStyle(QFrame::Sunken|QFrame::HLine);
        lay->addWidget(hline);
    }
    
    m_treeView = new KFileTreeView( page );
    m_treeView->addColumn( i18n("Albums") );
    m_treeView->setColumnWidthMode( 0, QListView::Maximum );
    m_treeView->setResizeMode( QListView::AllColumns );
    m_treeView->header()->hide();
    lay->addWidget(m_treeView);

    m_rootURL  = KURL(rootDir);
    m_startURL = KURL(startDir);
    m_rootURL.cleanPath();
    m_startURL.cleanPath();

    m_branch = m_treeView->addBranch(m_rootURL, i18n("My Albums"), false);
    m_branch->setChildRecurse(false);
    m_treeView->setDirOnlyMode(m_branch, true);

    KURL dirToList(m_startURL);

    m_urlsToList.clear();
    while (!dirToList.equals(m_rootURL, true))
    {
        m_urlsToList.push( dirToList );
        dirToList = dirToList.upURL();
        dirToList.cleanPath();
    }

    connect( m_treeView, SIGNAL( contextMenu( KListView *, QListViewItem *,
                                              const QPoint & )),
             SLOT( slotContextMenu( KListView *, QListViewItem *,
                                    const QPoint & )));
    connect( m_branch, SIGNAL( populateFinished( KFileTreeViewItem * )),
             SLOT( slotNextDirToList( KFileTreeViewItem * ) ));

    m_branch->setOpen(true);
}

DirSelectDialog::~DirSelectDialog()
{
}

void DirSelectDialog::openNextDir( KFileTreeViewItem* parent)
{
    KURL url = m_urlsToList.pop();
    kdDebug() << "listing " << url.prettyURL() << endl;

    QListViewItem * child = parent->firstChild();
    while( child ) {
        if (child->text(0) == url.fileName())
            break;
        child = child->nextSibling();
    }
    
    KFileTreeViewItem *item = (KFileTreeViewItem*)child;
    if ( item )
    {
        if ( !item->isOpen() )
            item->setOpen( true );
        else // already open -> go to next one
            slotNextDirToList( item );
    }
    else
    {
        kdWarning() << k_funcinfo <<  "Could not find item"
                    << endl;
        m_urlsToList.push(url);
    }
}

void DirSelectDialog::slotNextDirToList( KFileTreeViewItem* item )
{
    m_treeView->ensureItemVisible( item );
    QRect r = m_treeView->itemRect( item );
    if ( r.isValid() )
    {
        int x, y;
        m_treeView->viewportToContents( m_treeView->contentsX(), r.y(), x, y );
        m_treeView->setContentsPos( x, y );
    }

    if ( !m_urlsToList.isEmpty() )
        openNextDir( item );
    else
    {
        m_branch->disconnect( SIGNAL( populateFinished( KFileTreeViewItem * )),
                               this, SLOT( slotNextDirToList( KFileTreeViewItem *)));
        m_treeView->setCurrentItem( item );
        item->setSelected( true );
    }
}

void DirSelectDialog::slotContextMenu(KListView*, QListViewItem*,
                                      const QPoint& pos)
{
    QPopupMenu contextMenu(this);
    KAction *action = new KAction(i18n( "Create New Album" ),
                                  "albumfoldernew", 0, this,
                                  SLOT( slotUser1() ),
                                  &contextMenu);
    action->plug(&contextMenu);
    contextMenu.exec(pos);
}

void DirSelectDialog::slotUser1()
{
    QListViewItem* item = m_treeView->currentItem();
    if (!item)
        item = m_branch->root();

    KFileTreeViewItem* ftvItem = (KFileTreeViewItem*)item;
    if (!ftvItem)
        return;

    QString relPath = KURL::relativePath(m_rootURL.path(1),
                                         ftvItem->fileItem()->url().path(1));
    if (!relPath.startsWith("/"))
        relPath.prepend("/");
    relPath = QDir::cleanDirPath(relPath);
        
    bool ok;

#if KDE_IS_VERSION(3,2,0)
    QString newDir = KInputDialog::getText(i18n("New Album Name"),
                                           i18n("Creating New Album in 'My Albums%1'\n"
                                                "Enter Album Name")
                                           .arg(relPath),
                                           QString::null, &ok, this);
#else
    QString newDir = KLineEditDlg::getText(i18n("New Album Name"),
                                           i18n("Creating New Album in 'My Albums%1'\n"
                                                "Enter Album Name")
                                           .arg(relPath),
                                           QString::null, &ok, this);
#endif

    if (!ok)
        return;

    KURL url(ftvItem->fileItem()->url());
    url.addPath(newDir);

    KIO::mkdir(url);

    KURL dirToList(url);
    m_urlsToList.clear();
    while (!dirToList.equals(m_rootURL, true))
    {
        m_urlsToList.push( dirToList );
        dirToList = dirToList.upURL();
        dirToList.cleanPath();
    }

    m_branch->disconnect( SIGNAL( populateFinished( KFileTreeViewItem * )),
                          this, SLOT( slotNextDirToList( KFileTreeViewItem *)));
    connect( m_branch, SIGNAL( populateFinished( KFileTreeViewItem * )),
             SLOT( slotNextDirToList( KFileTreeViewItem * ) ));

    openNextDir(m_branch->root());
}

KURL DirSelectDialog::selectDir(const QString& rootDir, const QString& startDir,
                                QWidget* parent, QString header)
{
    DirSelectDialog dlg(rootDir, startDir, parent, header);

    if (dlg.exec() != QDialog::Accepted)
        return KURL();

    KFileTreeViewItem* ftvItem = (KFileTreeViewItem*) dlg.m_treeView->currentItem();
    if (!ftvItem)
        return KURL();

    return ftvItem->fileItem()->url();
}

#include "dirselectdialog.moc"
