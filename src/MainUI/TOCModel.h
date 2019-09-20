/************************************************************************
**
**  Copyright (C) 2016-2019 Kevin B Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef TOCMODEL_H
#define TOCMODEL_H

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
class TOCModel : public QStandardItemModel
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The model's parent.
     */
    TOCModel(QObject *parent = 0);

    /**
     * Sets the model's book.
     *
     * @param book The book whose model we will be building.
     */
    void SetBook(QSharedPointer<Book> book);

    /**
     * Translates a model index of an item into an URL
     * that points to the TOC item target.
     *
     * @param index The model index of an item.
     * @return The target URL of the item.
     */
    QString GetBookPathForIndex(const QModelIndex &index);

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
    struct TOCEntry {
        /**
         * The entry text that will be presented to the user.
         */
        QString text;

        /**
         * A Book Path representing the target of the src attribute
         * (note: there may be a fragment!).
         */
        QString target;

        /**
         * This entry's sub-entries (its children).
         */
        QList<TOCEntry> children;

        /**
         * If \c true, then this is an "invisible" root NCXEntry;
         * this entry has no text or target, only children.
         */
        bool is_root;

        TOCEntry() {
            is_root = false;
        }
    };

    /**
     * Reads the NCX file or Nav file, parses it and returns the root TOC
     * entry (that entry is the tree start). 
     * @return The root TOCEntry.
     */
    TOCEntry GetRootTOCEntry();

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
     * Parses the NCX source and returns the root TOC entry.
     *
     * @param ncx_source The NCX source code.
     * @return The root TOCEntry.
     */
    TOCEntry ParseNCX(const QString &ncx_source);

    /**
     * Parses an NCX navPoint element. Calls itself recursively
     * if there are navPoint children.
     *
     * @param ncx The QXmlStreamReader reading an NCX file positioned
     *            on a navPoint element.
     */
    TOCEntry ParseNavPoint(QXmlStreamReader &ncx);

    QString ConvertHREFToBookPath(const QString& ahref);

    /**
     * Builds the actual display model from the tree of TOCEntries.
     *
     * @param root_entry The root TOC entry.
     */
    void BuildModel(const TOCEntry &root_entry);

    /**
     * Adds the provided entry as an item child to the provided parent.
     * Calls itself recursively if the entry has children of it's own.
     */
    void AddEntryToParentItem(const TOCEntry &entry, QStandardItem *parent);


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The book whose model we are representing.
     */
    QSharedPointer<Book> m_Book;

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
    QFutureWatcher<TOCEntry> *m_TocRootWatcher;

    QString m_EpubVersion;
};


#endif // TOCMODEL_H
