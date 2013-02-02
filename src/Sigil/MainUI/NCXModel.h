/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef NCXMODEL_H
#define NCXMODEL_H

#include <QtCore/QFutureWatcher>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtGui/QStandardItemModel>

#include "BookManipulation/Book.h"

class NCXResource;
class QModelIndex;
class QStandardItem;
class QXmlStreamReader;
class QUrl;


/**
 * A hierarchical model for the NCX structure.
 * Meant to be used with a Qt View class (like QTreeView).
 */
class NCXModel : public QStandardItemModel
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The model's parent.
     */
    NCXModel(QObject *parent = 0);

    /**
     * Sets the model's book.
     *
     * @param book The book whose model we will be building.
     */
    void SetBook(QSharedPointer< Book > book);

    /**
     * Translates a model index of an item into an URL
     * that points to the TOC item target.
     *
     * @param index The model index of an item.
     * @return The target URL of the item.
     */
    QUrl GetUrlForIndex(const QModelIndex &index);

    /**
     * Forces the recreation of the model
     * from the information in the stored book.
     * The refresh is done asynchronously, meaning that this
     * function will return quickly and the actual refreshing will be
     * done in a separate thread. The "second part" of this process is
     * in RefreshEnd.
     * Can be called repeatedly while a refresh is still in progress.
     */
    void Refresh();

    /**
     * Represents a single entry in the NCX TOC.
     */
    struct NCXEntry {
        /**
         * The entry text that will be presented to the user.
         */
        QString text;

        /**
         * The raw value of the "src" attribute, pointing to
         * the actual target of the entry (note: there may be a fragment!).
         */
        QString target;

        /**
         * This entry's sub-entries (its children).
         */
        QList< NCXEntry > children;

        /**
         * If \c true, then this is an "invisible" root NCXEntry;
         * this entry has no text or target, only children.
         */
        bool is_root;

        NCXEntry() {
            is_root = false;
        }
    };

    /**
     * Reads the NCX file, parses it and returns the root NCX
     * entry (that entry is the tree start). It's basically
     * just a function composition of ParseNCX and GetNCXText.
     *
     * @return The root NCX entry.
     */
    NCXEntry GetRootNCXEntry();

signals:
    void RefreshDone();

private slots:

    /**
     * Completes the refresh process. It is called after the
     * background thread has finished processing the NCX and
     * the UI can now be updated.
     */
    void RefreshEnd();

private:

    /**
     * Reads the NCX file and returns the text in it.
     *
     * @return The NCX text.
     */
    QString GetNCXText();

    /**
     * Parses the NCX source and returns the root NCX entry.
     *
     * @param ncx_source The NCX source code.
     * @return The root NCX entry.
     */
    static NCXEntry ParseNCX(const QString &ncx_source);

    /**
     * Parses an NCX navPoint element. Calls itself recursively
     * if there are navPoint children.
     *
     * @param ncx The QXmlStreamReader reading an NCX file positioned
     *            on a navPoint element.
     */
    static NCXEntry ParseNavPoint(QXmlStreamReader &ncx);

    /**
     * Builds the actual display model from the tree of NCXEntries.
     *
     * @param root_entry The root NCX entry.
     */
    void BuildModel(const NCXEntry &root_entry);

    /**
     * Adds the provided entry as an item child to the provided parent.
     * Calls itself recursively if the entry has children of it's own.
     */
    static void AddEntryToParentItem(const NCXEntry &entry, QStandardItem *parent);


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The book whose model we are representing.
     */
    QSharedPointer< Book > m_Book;

    /**
     * If \c true, then a refresh operation is in progress.
     */
    bool m_RefreshInProgress;

    /**
     * Guards the use of the m_Book variable.
     */
    QMutex m_UsingBookMutex;

    /**
     * Watches the completion of the GetRootNCXEntry func
     * and signals the RefreshEnd func when the root NCX entry is ready.
     */
    QFutureWatcher< NCXEntry > &m_NcxRootWatcher;
};


#endif // NCXMODEL_H
