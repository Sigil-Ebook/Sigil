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
#ifndef BOOK_H
#define BOOK_H

#include <QHash>
#include <QUrl>
#include <QVariant>
#include <QObject>

class FolderKeeper;
class HTMLResource;
class Resource;
class OPFResource;
class NCXResource;

/**
 * Represents the book loaded in the current MainWindow instance
 * of Sigil. The book's resources are accessed through the FolderKeeper
 * instance.
 */
class Book : public QObject
{    
    Q_OBJECT

public:

    /**
     * Constructor.
     */
    Book();

    /**
     * Returns the base url of the book. 
     * This is the location of the text folder
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
    const FolderKeeper& GetFolderKeeper() const;

    /**
     * Returns the book's OPF file.
     * 
     * @return The OPF.
     */
    OPFResource& GetOPF();

    /**
     * Returns the book's OPF file as a const object.
     * 
     * @return The OPF.
     */
    const OPFResource& GetConstOPF() const;

    /**
     * Returns the book's NCX file.
     * 
     * @return The NCX.
     */
    NCXResource& GetNCX();

    /**
     * Returns the book's publication identifier.
     *
     * @return A string representing the publication identifier
     *         of the book. Used in the OPF file on epub export. 
     */
    QString GetPublicationIdentifier() const;

    /**
     * Returns the book's metadata.
     *
     * @return A hash representing the book's metadata. The keys are
     *         are the metadata names, and the values are the lists of
     *         metadata values for that metadata name.
     */
    QHash< QString, QList< QVariant > > GetMetadata() const;

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
     * @see FlowTab::SplitChapter, FlowTab::OldTabRequest,
     *      BookViewEditor::SplitChapter, MainWindow::CreateChapterBreakOldTab
     */
    HTMLResource& CreateChapterBreakOriginalResource( const QString &content, 
                                                      HTMLResource& originating_resource );

    /**
     * Creates new chapters/XHTML documents.
     * The only reason why we have an overload instead of just one function
     * with a default argument is because then Apple GCC 4.2 flakes out here.
     * 
     * @param new_chapters The contents of the new chapters.
     */
    void CreateNewChapters( const QStringList& new_chapters );

    /**
     * Creates new chapters/XHTML documents.
     *
     * @param new_chapters The contents of the new chapters.
     * @param html_updates The new chapters are updated with these updates.
     */
    void CreateNewChapters( const QStringList& new_chapters,
                            const QHash< QString, QString > &html_updates );

    /**
     * Merges the provided HTML resource with the previous one
     * in the reading order.
     *
     * @param html_resource The resource being merged.
     */
    void MergeWithPrevious( HTMLResource& html_resource );

    /**
     * Makes sure that all the resources have saved the state of 
     * their caches to the disk.
     */
    void SaveAllResourcesToDisk();

    /**
     * Returns the modified state of the book. A book
     * is loaded/created as not modified.
     *
     * @return \c true if the book has been modified.
     */
    bool IsModified() const;

    /**
     * Checks for the presence of obfuscated fonts in the book.
     *
     * @return \c true if the book has obfuscated fonts.
     */
    bool HasObfuscatedFonts() const ;

public slots:

    /**
     * Sets the modified state of the book.
     * @note \b Always use this function to change the state of
     *          m_IsModified variable since this will emit the 
     *          ModifiedStateChanged() signal if needed.
     *
     * @param modified The new modified state.
     */
    void SetModified( bool modified = true );

signals:

    /**
     * Emitted whenever the book's modified state changes.
     *
     * @param new_state The new modified state.
     */
    void ModifiedStateChanged( bool new_state );

private:

    /**
     * Syncs the content of one resource to the disk. 
     * @param resource The resource to be synced.
     */
    static void SaveOneResourceToDisk( Resource *resource );

    /**
     * Creates one new chapter/XHTML document.
     *
     * @param source The source code of the new chapter.
     * @param reading_order The reading order of the new chapter.
     * @param temp_folder_path The path to the temporary folder where the new file
     *                         will be created before being copied to the folderkeeper.
     */
    HTMLResource* CreateOneNewChapter( const QString &source,
                                       int reading_order,
                                       const QString &temp_folder_path );

    /**
     * Creates one new chapter/XHTML document.
     * The only reason why we have an overload instead of just one function
     * with a default argument is because then Apple GCC 4.2 flakes out here.
     * 
     * @param source The source code of the new chapter.
     * @param reading_order The reading order of the new chapter.
     * @param temp_folder_path The path to the temporary folder where the new file 
     *                         will be created before being copied to the folderkeeper.
     * @param html_updates Any reference updates that need to be performed.
     */
    HTMLResource* CreateOneNewChapter( const QString &source, 
                                       int reading_order, 
                                       const QString &temp_folder_path,
                                       const QHash< QString, QString > &html_updates );


    ////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ////////////////////////////

    /**
     * The FolderKeeper object that represents
     * this book's presence on the hard drive.
     */
    FolderKeeper &m_Mainfolder; 

    /**
     * A hash with meta information about the book. The keys are
     * are the metadata names, and the values are the lists of
     * metadata values for that metadata name.
     */
    QHash< QString, QList< QVariant > > m_Metadata;

    /**
     * Stores the modified state of the book.
     */
    bool m_IsModified;

};

#endif // BOOK_H


