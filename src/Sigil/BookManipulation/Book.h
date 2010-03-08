/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#ifndef BOOK_H
#define BOOK_H

#include "../BookManipulation/FolderKeeper.h"
#include <QHash>
#include <QUrl>
#include <QVariant>


/**
 * Represents the book loaded in the current MainWindow instance
 * of Sigil. The book's resources are accessed through the FolderKeeper
 * instance.
 */
class Book
{    

public:

    /**
     * Constructor.
     */
    Book();

    /**
     * Copy constructor.
     */
    Book( const Book& other );

    /**
     * Assignment operator.
     */
    Book& operator = ( const Book& other );

    /**
     * Returns the base url of the book. 
     * This is the location to the text folder
     * within the main folder.
     */
    QUrl GetBaseUrl() const;

    /**
     * Returns the FolderKeeper instance.
     *
     * @return A reference to the FolderKeeper instance of the Book.
     */
    FolderKeeper& GetFolderKeeper();

    /**
    * Returns the FolderKeeper instance.
    *
    * @return A const reference to the FolderKeeper instance of the Book.
    */
    const FolderKeeper& GetConstFolderKeeper();

    /**
     * Returns the book's publication identifier.
     *
     * @return A string representing the publication identifier
     *         of the book. Used in the OPF file on epub export. 
     */
    QString GetPublicationIdentifier();

    /**
     * Returns the book's metadata.
     *
     * @return A hash representing the book's metadata. The keys are
     *         are the metadata names, and the values are the lists of
     *         metadata values for that metadata name.
     */
    QHash< QString, QList< QVariant > > GetMetadata();

    /**
     * Replaces the book's current meta information with the received metadata.
     *
     * @param metadata A hash with meta information about the book.The keys are
     *                 are the metadata names, and the values are the lists of
     *                 metadata values for that metadata name.
     */
    void SetMetadata( const QHash< QString, QList< QVariant > > metadata );

    /**
     * Creates a new HTMLResource file with no stored data. 
     * The file on disk has only placeholder text.
     *
     * @return A reference to the created HTMLResource file.
     */
    HTMLResource& CreateNewHTMLFile();

    /**
     * Creates a new HTMLResource file with a basic XHTML structure. 
     * The file on disk has only placeholder text.
     */
    void CreateEmptyHTMLFile();

    /**
     * Creates a new CSSResource file with no stored data. 
     * The file on disk is empty.
     */
    void CreateEmptyCSSFile();

    /**
     * Creates an "old" resource from a chapter breaking operation. 
     * The chapter break operation actually creates a new resource
     * from the chapter content up to the chapter break point.
     *
     * @param content The content of the "old" tab/resource.
     * @param originating_resource  The original resource from which the content
     *                              was extracted to create the "old" tab/resource.
     * @return A reference to the newly created "old" tab/resource.
     * @see FlowTab::InsertChapterBreak, FlowTab::OldTabRequest,
     *      BookViewEditor::SplitChapter, MainWindow::CreateChapterBreakOldTab
     */
    HTMLResource& CreateChapterBreakOriginalResource( const QString &content, 
                                                      HTMLResource& originating_resource );

    /**
     * Makes sure that all the resources have saved the state of 
     * their caches to the disk.
     */
    void SaveAllResourcesToDisk();

private:

    /**
     * Syncs the content of one resource to the disk. 
     * @param resource The resource to be synced.
     */
    static void SaveOneResourceToDisk( Resource *resource );

    /**
     * The FolderKeeper object that represents
     * this book's presence on the hard drive.
     */
    FolderKeeper m_Mainfolder; 

    /**
     * The book's publication identifier. 
     * Currently an UUID.
     */
    QString m_PublicationIdentifier;

    /**
     * A hash with meta information about the book.The keys are
     * are the metadata names, and the values are the lists of
     * metadata values for that metadata name.
     */
    QHash< QString, QList< QVariant > > m_Metadata;

};

#endif // BOOK_H


