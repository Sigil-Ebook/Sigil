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

#include <boost/bind/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <QtCore/QtCore>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/Metadata.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Importers/ImportHTML.h"
#include "Misc/HTMLEncodingResolver.h"
#include "Misc/SettingsStore.h"
#include "Misc/TempFolder.h"
#include "Misc/Utility.h"
#include "ResourceObjects/CSSResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "SourceUpdates/PerformHTMLUpdates.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

using boost::tie;

// Constructor;
// The parameter is the file to be imported
ImportHTML::ImportHTML(const QString &fullfilepath)
    :
    Importer(fullfilepath),
    m_IgnoreDuplicates(false),
    m_CachedSource(QString())
{
}


void ImportHTML::SetBook(QSharedPointer< Book > book, bool ignore_duplicates)
{
    m_Book = book;
    m_IgnoreDuplicates = ignore_duplicates;
}


XhtmlDoc::WellFormedError ImportHTML::CheckValidToLoad()
{
    // For HTML & XML documents we will perform a well-formed check
    return XhtmlDoc::WellFormedErrorForSource(LoadSource());
}


// Reads and parses the file
// and returns the created Book
QSharedPointer< Book > ImportHTML::GetBook()
{
    shared_ptr< xc::DOMDocument > document = XhtmlDoc::LoadTextIntoDocument(LoadSource());
    LoadMetadata(*document);
    UpdateFiles(CreateHTMLResource(), *document, LoadFolderStructure(*document));
    return m_Book;
}


// Loads the source code into the Book
QString ImportHTML::LoadSource()
{
    SettingsStore ss;

    if (m_CachedSource.isNull()) {
        if (!Utility::IsFileReadable(m_FullFilePath)) {
            boost_throw(CannotReadFile() << errinfo_file_fullpath(m_FullFilePath.toStdString()));
        }

        m_CachedSource = HTMLEncodingResolver::ReadHTMLFile(m_FullFilePath);
        m_CachedSource = CleanSource::CharToEntity(m_CachedSource);

        if (ss.cleanOn() & CLEANON_OPEN) {
            m_CachedSource = CleanSource::Clean(XhtmlDoc::ResolveCustomEntities(m_CachedSource));
        }
    }

    return m_CachedSource;
}


// Searches for meta information in the HTML file
// and tries to convert it to Dublin Core
void ImportHTML::LoadMetadata(const xc::DOMDocument &document)
{
    QList< xc::DOMElement * > metatags = XhtmlDoc::GetTagMatchingDescendants(document, "meta");
    QList< Metadata::MetaElement > metadata;

    for (int i = 0; i < metatags.count(); ++i) {
        xc::DOMElement &element = *metatags.at(i);
        Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata(element);

        if (!book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty()) {
            metadata.append(book_meta);
        }
    }

    m_Book->SetMetadata(metadata);
}


HTMLResource &ImportHTML::CreateHTMLResource()
{
    TempFolder tempfolder;
    QString fullfilepath = tempfolder.GetPath() + "/" + QFileInfo(m_FullFilePath).fileName();
    Utility::WriteUnicodeTextFile("TEMP_SOURCE", fullfilepath);
    HTMLResource &resource = *qobject_cast< HTMLResource * >(
                                 &m_Book->GetFolderKeeper().AddContentFileToFolder(fullfilepath));
    return resource;
}


void ImportHTML::UpdateFiles(HTMLResource &html_resource,
                             xc::DOMDocument &document,
                             const QHash< QString, QString > &updates)
{
    Q_ASSERT(&html_resource != NULL);
    QHash< QString, QString > html_updates;
    QHash< QString, QString > css_updates;
    tie(html_updates, css_updates, boost::tuples::ignore) =
        UniversalUpdates::SeparateHtmlCssXmlUpdates(updates);
    QList< Resource * > all_files = m_Book->GetFolderKeeper().GetResourceList();
    int num_files = all_files.count();
    QList< CSSResource * > css_resources;

    for (int i = 0; i < num_files; ++i) {
        Resource *resource = all_files.at(i);

        if (resource->Type() == Resource::CSSResourceType) {
            css_resources.append(qobject_cast< CSSResource * >(resource));
        }
    }

    QFutureSynchronizer<void> sync;
    sync.addFuture(QtConcurrent::map(css_resources,
                                     boost::bind(UniversalUpdates::LoadAndUpdateOneCSSFile, _1, css_updates)));
    html_resource.SetText(XhtmlDoc::GetDomDocumentAsString(*PerformHTMLUpdates(document, html_updates, css_updates)().get()));
    sync.waitForFinished();
}


// Loads the referenced files into the main folder of the book;
// as the files get a new name, the references are updated
QHash< QString, QString > ImportHTML::LoadFolderStructure(const xc::DOMDocument &document)
{
    QFutureSynchronizer< QHash< QString, QString > > sync;
    sync.addFuture(QtConcurrent::run(this, &ImportHTML::LoadMediaFiles,     &document));
    sync.addFuture(QtConcurrent::run(this, &ImportHTML::LoadStyleFiles, &document));
    sync.waitForFinished();
    QList< QFuture< QHash< QString, QString > > > futures = sync.futures();
    int num_futures = futures.count();
    QHash< QString, QString > updates;

    for (int i = 0; i < num_futures; ++i) {
        updates.unite(futures.at(i).result());
    }

    return updates;
}


QHash< QString, QString > ImportHTML::LoadMediaFiles(const xc::DOMDocument *document)
{
    QStringList file_paths = XhtmlDoc::GetMediaPathsFromMediaChildren(*document, IMAGE_TAGS + VIDEO_TAGS + AUDIO_TAGS);
    QHash< QString, QString > updates;
    QDir folder(QFileInfo(m_FullFilePath).absoluteDir());
    QStringList current_filenames = m_Book->GetFolderKeeper().GetAllFilenames();
    // Load the media files (images, video, audio) into the book and
    // update all references with new urls
    foreach(QString file_path, file_paths) {
        try {
            QString filename = QFileInfo(file_path).fileName();
            QString fullfilepath  = QFileInfo(folder, file_path).absoluteFilePath();
            QString newpath;

            if (m_IgnoreDuplicates && current_filenames.contains(filename)) {
                newpath = "../" + m_Book->GetFolderKeeper().GetResourceByFilename(filename).GetRelativePathToOEBPS();
            } else {
                newpath       = "../" + m_Book->GetFolderKeeper()
                                .AddContentFileToFolder(fullfilepath).GetRelativePathToOEBPS();
            }

            updates[ fullfilepath ] = newpath;
        } catch (FileDoesNotExist &) {
            // Do nothing. If the referenced file does not exist,
            // well then we don't load it.
            // TODO: log this.
        }
    }
    return updates;
}


QHash< QString, QString > ImportHTML::LoadStyleFiles(const xc::DOMDocument *document)
{
    QList< xc::DOMElement * > link_nodes = XhtmlDoc::GetTagMatchingDescendants(*document, "link");
    QHash< QString, QString > updates;
    QStringList current_filenames = m_Book->GetFolderKeeper().GetAllFilenames();

    for (int i = 0; i < link_nodes.count(); ++i) {
        xc::DOMElement &element = *link_nodes.at(i);
        Q_ASSERT(&element);
        QDir folder(QFileInfo(m_FullFilePath).absoluteDir());
        QString relative_path = Utility::URLDecodePath(XtoQ(element.getAttribute(QtoX("href"))));
        QFileInfo file_info(folder, relative_path);

        if (file_info.suffix().toLower() == "css") {
            try {
                QString newpath;

                if (m_IgnoreDuplicates && current_filenames.contains(file_info.fileName())) {
                    newpath = "../" + m_Book->GetFolderKeeper().GetResourceByFilename(file_info.fileName()).GetRelativePathToOEBPS();
                } else {
                    newpath = "../" + m_Book->GetFolderKeeper().AddContentFileToFolder(
                                  file_info.absoluteFilePath()).GetRelativePathToOEBPS();
                }

                updates[ relative_path ] = newpath;
            } catch (FileDoesNotExist &) {
                // Do nothing. If the referenced file does not exist,
                // well then we don't load it.
                // TODO: log this.
            }
        }
    }

    return updates;
}



