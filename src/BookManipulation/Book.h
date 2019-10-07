/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON, Canada 
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

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include "ResourceObjects/OPFParser.h"
#include "BookManipulation/XhtmlDoc.h"
#include "ResourceObjects/Resource.h"

class CSSResource;
class SVGResource;
class FolderKeeper;
class HTMLResource;
class NCXResource;
class OPFResource;
class Resource;

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

    ~Book();

    /**
     * Returns the FolderKeeper instance.
     *
     * @return A reference to the FolderKeeper instance of the Book.
     */
    FolderKeeper *GetFolderKeeper();

    /**
     * Returns the FolderKeeper instance.
     *
     * @return A const reference to the FolderKeeper instance of the Book.
     */
    const FolderKeeper *GetFolderKeeper() const;

    /**
     * Returns the book's OPF file.
     *
     * @return The OPF.
     */
    OPFResource *GetOPF();

    /**
     * Returns the book's OPF file as a const object.
     *
     * @return The OPF.
     */
    const OPFResource *GetConstOPF() const;

    /**
     * Returns the book's NCX file.
     *
     * @return The NCX.
     */
    NCXResource *GetNCX();

    const NCXResource *GetConstNCX() const;

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
     * @return A list of the book's metadata.
     */
    QList<MetaEntry> GetMetadata() const;

    /**
     * Returns the values for a specific metadata name.
     *
     * @return A list of values
     */
    QList<QVariant> GetMetadataValues(QString text) const;

    /**
     * Replaces the book's current meta information with the received metadata.
     *
     * @param metadata A hash with meta information about the book.The keys are
     *                 are the metadata names, and the values are the lists of
     *                 metadata values for that metadata name.
     */
    void SetMetadata(const QList<MetaEntry> & metadata);

    QString GetFirstUniqueSectionName(QString extension = QString());

    /**
     * Creates a new HTMLResource file with no stored data.
     * The file on disk has only placeholder text.
     *
     * @return A reference to the created HTMLResource file.
     */
    HTMLResource *CreateNewHTMLFile();

    /**
     * Moves the first resource to after the second resource
     */
    void MoveResourceAfter(HTMLResource *from_resource, HTMLResource *to_resource);

    /**
     * Creates a new HTMLResource file with a basic XHTML structure.
     * The file on disk has only placeholder text.
     */
    HTMLResource *CreateEmptyHTMLFile();

    /**
     * Creates a new HTML Nav file with a basic nav structure.
     */
    HTMLResource *CreateEmptyNavFile(bool update_opf = false);

    /**
     * Creates a new HTMLResource file with a basic XHTML structure
     * inserted after the given resource.
     * The file on disk has only placeholder text.
     */
    HTMLResource *CreateEmptyHTMLFile(HTMLResource *resource);

    /**
     * Creates a new CSSResource file with no stored data.
     * The file on disk is empty.
     */
    CSSResource *CreateEmptyCSSFile();

    SVGResource *CreateEmptySVGFile();

    HTMLResource *CreateHTMLCoverFile(QString text);

    CSSResource *CreateHTMLTOCCSSFile();
    CSSResource *CreateIndexCSSFile();

    /**
     * Creates an "old" resource from a section breaking operation.
     * The section break operation actually creates a new resource
     * from the section content up to the section break point.
     *
     * @param content The content of the "old" tab/resource.
     * @param originating_resource  The original resource from which the content
     *                              was extracted to create the "old" tab/resource.
     * @return A reference to the newly created "old" tab/resource.
     * @see FlowTab::SplitSection, FlowTab::OldTabRequest,
     *      BookViewEditor::SplitSection, MainWindow::CreateSectionBreakOldTab
     */
    HTMLResource *CreateSectionBreakOriginalResource(const QString &content,
            HTMLResource *originating_resource);

    /**
     * Creates new sections/XHTML documents.
     * The only reason why we have an overload instead of just one function
     * with a default argument is because then Apple GCC 4.2 flakes out here.
     *
     * @param new_sections The contents of the new sections.
     * @param originating_resource The original HTML section that sections
     * will be created after.
     */
    void CreateNewSections(const QStringList &new_sections,
                           HTMLResource *originalResource);

    /**
     * Returns the previous resource, or the same resource if at top of folder
     *
     * @param resource The previous resource
     */
    Resource *PreviousResource(Resource *resource);

    QHash <QString, QList<XhtmlDoc::XMLElement>> GetLinkElements();
    static std::tuple<QString, QList<XhtmlDoc::XMLElement>> GetLinkElementsInHTMLFileMapped(HTMLResource *html_resource);

    QStringList GetStyleUrlsInHTMLFiles();
    static std::tuple<QString, QStringList> GetStyleUrlsInHTMLFileMapped(HTMLResource *html_resource);
    QHash<QString, QStringList> GetIdsInHTMLFiles();
    static std::tuple<QString, QStringList> GetIdsInHTMLFileMapped(HTMLResource *html_resource);
    QStringList GetIdsInHTMLFile(HTMLResource *html_resource);

    QStringList GetIdsInHrefs();
    QHash<QString, QStringList> GetHrefsInHTMLFiles();
    static std::tuple<QString, QStringList> GetHrefsInHTMLFileMapped(HTMLResource *html_resource);

    QStringList GetClassesInHTMLFile(HTMLResource* html_resource);

    QSet<QString> GetWordsInHTMLFiles();
    static QStringList GetWordsInHTMLFileMapped(HTMLResource *html_resource);

    QHash<QString, int> GetUniqueWordsInHTMLFiles();

    QHash<QString, QStringList> GetStylesheetsInHTMLFiles();
    static std::tuple<QString, QStringList> GetStylesheetsInHTMLFileMapped(HTMLResource *html_resource);
    QStringList GetStylesheetsInHTMLFile(HTMLResource *html_resource);

    QHash<QString, QStringList> GetImagesInHTMLFiles();
    QHash<QString, QStringList> GetVideoInHTMLFiles();
    QHash<QString, QStringList> GetAudioInHTMLFiles();

    QHash<QString, QStringList> GetHTMLFilesUsingMedia();
    QHash<QString, QStringList> GetHTMLFilesUsingImages();
    static std::tuple<QString, QStringList> GetMediaInHTMLFileMapped(HTMLResource *html_resource);
    static std::tuple<QString, QStringList> GetImagesInHTMLFileMapped(HTMLResource *html_resource);
    static std::tuple<QString, QStringList> GetVideoInHTMLFileMapped(HTMLResource *html_resource);
    static std::tuple<QString, QStringList> GetAudioInHTMLFileMapped(HTMLResource *html_resource);

    QList<HTMLResource *> GetNonWellFormedHTMLFiles();

    QHash<QString, int> CountAllLinksInHTML();

    /**
     * Merges two or more html resources together in order into the first resource in the list.
     * If the merge fails, returns resource which caused the failure, otherwise returns null.
     */
    Resource *MergeResources(QList<Resource *> resources);

    QList <Resource *> GetAllResources();

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
     * Returns whether or not a resource's data is well formed
     * whether or not it is open in a tab
     *
     * @return true if well formed
     */
    bool IsDataWellFormed(HTMLResource *html_resource);

    /**
     * Returns whether or not a resource's data is well formed
     * whether or not its open in a tab by looking at the disk.
     *
     * @return true if well formed
     */
    bool IsDataOnDiskWellFormed(HTMLResource *html_resource);

    /**
     * Reformats All the book's html resources using either
     * the 'Mend' or "Mend and Repair' CleanSource procedures.
     */
    void ReformatAllHTML(bool to_valid);

    /**
     * Checks for the presence of obfuscated fonts in the book.
     *
     * @return \c true if the book has obfuscated fonts.
     */
    bool HasObfuscatedFonts() const ;

    QList<HTMLResource *> GetHTMLResources();

    /** Check for undefined url fragments in all HTMLResources.
     * Return a tuple result (undefined fragments exist?, href, source-file)
     */
    std::tuple<bool, QString, QString> HasUndefinedURLFragments();

    /**
     * Get all href values in all relative links from supplied HTMLResources
     * using QtConcurrent. Return hash of hrefs keyed on filename.
     */
    static QHash<QString, QStringList> GetRelLinksInAllFiles(const QList<HTMLResource *> &html_resources);

    /**
     * Get all id values from supplied HTMLResources using QtConcurrent.
     * Return hash of ids keyed on filename.
     */
    static QHash<QString, QStringList> GetIDsInAllFiles(const QList<HTMLResource *> &html_resources);

public slots:

    /**
     * Sets the modified state of the book.
     * @note \b Always use this function to change the state of
     *          m_IsModified variable since this will emit the
     *          ModifiedStateChanged() signal if needed.
     *
     * @param modified The new modified state.
     */
    void SetModified(bool modified = true);

    void ResourceUpdatedFromDisk(Resource *resource);

signals:

    /**
     * Emitted whenever the book's modified state changes.
     *
     * @param new_state The new modified state.
     */
    void ModifiedStateChanged(bool new_state);

    void ResourceUpdatedFromDiskRequest(Resource *resource);

private:

    // Describe a new section.
    //
    // This is needed because QtConcurrent only seems to accept functions with
    //a maximum of 5 arguments.
    struct NewSection {
        // The source code of the new section.
        QString source;

        // The reading order of the new section.
        int reading_order;

        // The path to the temporary folder where the new file will be
        // created before being copied to the folderkeeper.
        QString temp_folder_path;

        // Prefix used when creating the filename.
        QString new_file_prefix;

        // Number to use as the suffix of the filename.
        int file_suffix;

        // Extension to use for the filename.
        QString file_extension;

    };

    // Describe a newly created section.
    struct NewSectionResult {
        // Chatper that was created.
        HTMLResource *created_section;

        // Position in the reading order of this section.
        int reading_order;
    };

    /**
     * Syncs the content of one resource to the disk.
     * @param resource The resource to be synced.
     */
    static void SaveOneResourceToDisk(Resource *resource);

    /**
     * Creates one new section/XHTML document.
     *
     * @param section_info New section to create.
     */
    NewSectionResult CreateOneNewSection(NewSection section_info);

    /**
     * Creates one new section/XHTML document.
     * The only reason why we have an overload instead of just one function
     * with a default argument is because then Apple GCC 4.2 flakes out here.
     *
     * @param section_info New section to create.
     * @param html_updates Any reference updates that need to be performed.
     */
    NewSectionResult CreateOneNewSection(NewSection section_info,
                                         const QHash<QString, QString> &html_updates);

    /**
     * Get all href values in all relative links from one HTMLResource
     * Return QPair(filename, hrefs).
     */
    static QPair<QString, QStringList> GetRelLinksInOneFile(HTMLResource *html_resource);

    /**
     * Get all id values from one HTMLResource. Return QPair(filename, ids).
     */
    static QPair<QString, QStringList> GetOneFileIDs(HTMLResource *html_resource);


    ////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ////////////////////////////

    /**
     * The FolderKeeper object that represents
     * this book's presence on the hard drive.
     */
    FolderKeeper *m_Mainfolder;


    /**
     * Stores the modified state of the book.
     */
    bool m_IsModified;

};

#endif // BOOK_H


