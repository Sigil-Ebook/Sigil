/************************************************************************
**
**  Copyright (C) 2015-2019  Kevin B. Hendricks Stratford, ON, Canada 
**  Copyright (C) 2009-2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QtCore>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProgressDialog>

#include "BookManipulation/Book.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/FolderKeeper.h"
#include "Misc/GumboInterface.h"
#include "Misc/TempFolder.h"
#include "Misc/Utility.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/Landmarks.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"
#include "SourceUpdates/AnchorUpdates.h"
#include "SourceUpdates/PerformHTMLUpdates.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "Misc/SettingsStore.h"

static const QString FIRST_CSS_NAME   = "Style0001.css";
static const QString FIRST_SVG_NAME   = "Image0001.svg";
static const QString PLACEHOLDER_TEXT = "PLACEHOLDER";
static const QString EMPTY_HTML_FILE  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                                        "  \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n\n"
                                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                                        "<head>\n"
                                        "  <title></title>\n"
                                        "</head>\n\n"
                                        "<body>\n"

                                        // The "nbsp" is here so that the user starts writing
                                        // inside the <p> element; if it's not here, webkit
                                        // inserts text _outside_ the <p> element
                                        "  <p>&nbsp;</p>\n"
                                        "</body>\n"
                                        "</html>";

static const QString EMPTY_HTML5_FILE  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                                        "<!DOCTYPE html>\n\n"
                                        "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\">\n"
                                        "<head>\n"
                                        "  <title></title>\n"
                                        "</head>\n\n"
                                        "<body>\n"

                                        // The numeric entity for nbsp is here so that the user starts writing
                                        // inside the <p> element; if it's not here, webkit
                                        // inserts text _outside_ the <p> element, Epub3 requires numeric entities
                                        "  <p>&#160;</p>\n"
                                        "</body>\n"
                                        "</html>";

const QString SGC_NAV_CSS_FILENAME = "sgc-nav.css";

const QString EMPTY_NAV_FILE_START = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" "
    "lang=\"%1\" xml:lang=\"%2\">\n"
    "<head>\n"
    "  <title></title>\n"
    "  <meta charset=\"utf-8\" />\n"
    "  <link href=\"%3\" rel=\"stylesheet\" type=\"text/css\"/>"
    "</head>\n"
    "<body epub:type=\"frontmatter\">\n";

const QString EMPTY_NAV_FILE_TOC = 
    "  <nav epub:type=\"toc\" id=\"toc\">\n"
    "    <h1>%1</h1>\n"
    "    <ol>\n"
    "      <li>\n"
    "        <a href=\"%2\">%3</a>\n"
    "      </li>\n"
    "    </ol>\n"
    "  </nav>\n";

const QString EMPTY_NAV_FILE_LANDMARKS = 
    "  <nav epub:type=\"landmarks\" id=\"landmarks\" hidden=\"\">\n"
    "    <h2>%1</h2>\n"
    "    <ol>\n"
    "      <li>\n"
    "        <a epub:type=\"toc\" href=\"#toc\">%2</a>\n"
    "      </li>\n"
    "    </ol>\n"
    "  </nav>\n";

const QString EMPTY_NAV_FILE_END = 
    "</body>\n"
    "</html>";

static const QString SGC_NAV_CSS_FILE =
    "nav#landmarks {\n"
    "    display:none;\n"
    "}\n\n"
    "nav#page-list {\n"
    "    display:none;\n"
    "}\n\n"
    "ol {\n"
    "    list-style-type: none;\n"
    "}\n\n";


const QString HTML_NAV_FILENAME = "nav.xhtml";

static const QString SGC_TOC_CSS_FILE =
    "div.sgc-toc-title {\n"
    "    font-size: 2em;\n"
    "    font-weight: bold;\n"
    "    margin-bottom: 1em;\n"
    "    text-align: center;\n"
    "}\n\n"
    "div.sgc-toc-level-1 {\n"
    "    margin-left: 0em;\n"
    "}\n\n"
    "div.sgc-toc-level-2 {\n"
    "    margin-left: 2em;\n"
    "}\n\n"
    "div.sgc-toc-level-3 {\n"
    "    margin-left: 2em;\n"
    "}\n\n"
    "div.sgc-toc-level-4 {\n"
    "    margin-left: 2em;\n"
    "}\n\n"
    "div.sgc-toc-level-5 {\n"
    "    margin-left: 2em;\n"
    "}\n\n"
    "div.sgc-toc-level-6 {\n"
    "    margin-left: 2em;\n"
    "}\n";

static const QString SGC_INDEX_CSS_FILE =
    "div.sgc-index-title {\n"
    "    font-size: 2em;\n"
    "    font-weight: bold;\n"
    "    margin-bottom: 1em;\n"
    "    text-align: center;\n"
    "}\n\n"
    "div.sgc-index-body {\n"
    "    margin-left: -2em;\n"
    "}\n\n"
    "div.sgc-index-entry {\n"
    "    margin-top: 0em;\n"
    "    margin-bottom: 0.5em;\n"
    "    margin-left: 3.5em;\n"
    "    text-indent: -1.5em;\n"
    "}\n\n"
    "div.sgc-index-new-letter {\n"
    "    margin-top: 1.5em;\n"
    "    margin-left: 1.3em;\n"
    "    margin-bottom: 0.5em;\n"
    "    font-size: 1.5em;\n"
    "    font-weight: bold;\n"
    "    border-bottom: solid black 4px;\n"
    "    width: 50%;\n"
    "}\n";

const QString HTML_COVER_SOURCE =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
    "\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "<head>\n"
    "  <title>Cover</title>\n"
    "</head>\n"
    ""
    "<body>\n"
    "  <div style=\"text-align: center; padding: 0pt; margin: 0pt;\">\n"
    "    <svg xmlns=\"http://www.w3.org/2000/svg\" height=\"100%\" preserveAspectRatio=\"xMidYMid meet\" version=\"1.1\" viewBox=\"0 0 SGC_IMAGE_WIDTH SGC_IMAGE_HEIGHT\" width=\"100%\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
    "      <image width=\"SGC_IMAGE_WIDTH\" height=\"SGC_IMAGE_HEIGHT\" xlink:href=\"SGC_IMAGE_FILENAME\"/>\n"
    "    </svg>\n"
    "  </div>\n"
    "</body>\n"
    "</html>\n";

const QString HTML5_COVER_SOURCE =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
    "<!DOCTYPE html>\n\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\">\n"
    "<head>\n"
    "  <title>Cover</title>\n"
    "</head>\n"
    ""
    "<body>\n"
    "  <div style=\"text-align: center; padding: 0pt; margin: 0pt;\">\n"
    "    <svg xmlns=\"http://www.w3.org/2000/svg\" height=\"100%\" preserveAspectRatio=\"xMidYMid meet\" version=\"1.1\" viewBox=\"0 0 SGC_IMAGE_WIDTH SGC_IMAGE_HEIGHT\" width=\"100%\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
    "      <image width=\"SGC_IMAGE_WIDTH\" height=\"SGC_IMAGE_HEIGHT\" xlink:href=\"SGC_IMAGE_FILENAME\"/>\n"
    "    </svg>\n"
    "  </div>\n"
    "</body>\n"
    "</html>\n";

Book::Book()
    :
    m_Mainfolder(new FolderKeeper(this)),
    m_IsModified(false)
{
}

Book::~Book()
{
    delete m_Mainfolder;
}


FolderKeeper *Book::GetFolderKeeper()
{
    return m_Mainfolder;
}


const FolderKeeper *Book::GetFolderKeeper() const
{
    return m_Mainfolder;
}



OPFResource *Book::GetOPF()
{
    return m_Mainfolder->GetOPF();
}


const OPFResource *Book::GetConstOPF() const
{
    return m_Mainfolder->GetOPF();
}


NCXResource *Book::GetNCX()
{
    return m_Mainfolder->GetNCX();
}


const NCXResource *Book::GetConstNCX() const
{
    return m_Mainfolder->GetNCX();
}


QString Book::GetPublicationIdentifier() const
{
    return GetConstOPF()->GetMainIdentifierValue();
}


QList<MetaEntry> Book::GetMetadata() const
{
    return GetConstOPF()->GetDCMetadata();
}

QList<QVariant> Book::GetMetadataValues(QString text) const
{
    return GetConstOPF()->GetDCMetadataValues(text);
}

void Book::SetMetadata(const QList<MetaEntry> &metadata)
{
    GetOPF()->SetDCMetadata(metadata);
    SetModified(true);
}

QString Book::GetFirstUniqueSectionName(QString extension)
{
    // If not files just return the default first name
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);

    if (html_resources.count() < 1) {
        return FIRST_SECTION_NAME;
    }

    // Get the extension of the current file
    QString first_html_file = html_resources.first()->Filename();
    QString first_html_filename = first_html_file.left(first_html_file.lastIndexOf("."));

    if (extension.isEmpty()) {
        extension = first_html_file.right(first_html_file.length() - first_html_file.lastIndexOf("."));
        // do not create new extensions with .xml or any other strange extension that are vague
        // ie. xhtml is xml but xml need not be xhtml
        if ((extension != ".xhtml") && (extension != ".htm") && (extension != ".html")) {
            extension = ".xhtml";
        }
        // If no extension use the default first name extension
        if (extension.isEmpty()) {
            extension = FIRST_SECTION_NAME;
            extension = extension.right(extension.length() - extension.lastIndexOf("."));
        }
    }

    QString filename = FIRST_SECTION_PREFIX + extension;
    return m_Mainfolder->GetUniqueFilenameVersion(filename);
}

QList<HTMLResource *> Book::GetHTMLResources()
{
    return m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
}

HTMLResource *Book::CreateNewHTMLFile(const QString &folder_path)
{
    TempFolder tempfolder;
    QString fullfilepath = tempfolder.GetPath() + "/" + GetFirstUniqueSectionName();
    Utility::WriteUnicodeTextFile(PLACEHOLDER_TEXT, fullfilepath);
    Resource * resource = m_Mainfolder->AddContentFileToFolder(fullfilepath, 
                                                               true, 
							       QString("application/xhtml+xml"),
							       QString(),
							       folder_path);
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
    SetModified(true);
    return html_resource;
}


HTMLResource *Book::CreateEmptyHTMLFile(const QString &folderpath)
{
    HTMLResource *html_resource = CreateNewHTMLFile(folderpath);
    QString version = html_resource->GetEpubVersion();
    if (version.startsWith('2')) {
        html_resource->SetText(EMPTY_HTML_FILE);
    } else {
        html_resource->SetText(EMPTY_HTML5_FILE);
    }
    SetModified(true);
    return html_resource;
}


HTMLResource *Book::CreateEmptyNavFile(bool update_opf,
				       const QString &folderpath,
				       const QString &navname,
				       const QString &first_textdir)
{
    bool found_css = false;
    Resource * styleresource = NULL;
    QList<Resource*> resources = GetFolderKeeper()->GetResourceTypeAsGenericList<CSSResource>(false);
    foreach(Resource *resource, resources) {
        if (resource->Filename() == SGC_NAV_CSS_FILENAME) {
	    styleresource = resource;
            found_css = true;
            break;
        }
    }
    // If NAV CSS file does not exist look for a default file
    // in preferences directory and if none create one.
    if (!found_css) {
        TempFolder tempfolder;
        QString css_path = Utility::DefinePrefsDir() + "/" + SGC_NAV_CSS_FILENAME;
        if (!QFile::exists(css_path)) {
            css_path = tempfolder.GetPath() + "/" + SGC_NAV_CSS_FILENAME;
            Utility::WriteUnicodeTextFile(SGC_NAV_CSS_FILE, css_path);
        }
        styleresource = m_Mainfolder->AddContentFileToFolder(css_path, 
							     update_opf, 
							     "text/css");
        CSSResource *css_resource = qobject_cast<CSSResource *> (styleresource);
        // Need to make sure InitialLoad is done in newly added css resource object to prevent
        // blank css issues after a save to disk
        if (css_resource) css_resource->InitialLoad();       
    }

    TempFolder tempfolder;
    QString fullfilepath = tempfolder.GetPath() + "/" + navname;
    Utility::WriteUnicodeTextFile(PLACEHOLDER_TEXT, fullfilepath);
    Resource * resource = m_Mainfolder->AddContentFileToFolder(fullfilepath,
							       update_opf, 
							       "application/xhtml+xml",
							       QString(),
							       folderpath);

    // get the informtion we need to correctly fill the template
    QString navbookpath = resource->GetRelativePath();
    QString navstylebookpath = styleresource->GetRelativePath();
    QString textdir = GetFolderKeeper()->GetDefaultFolderForGroup("Text");
    if (first_textdir != "\\") textdir = first_textdir;
    QString first_section_bookpath = FIRST_SECTION_NAME;
    if (!textdir.isEmpty()) first_section_bookpath = textdir + "/" + FIRST_SECTION_NAME;
    QString stylehref = Utility::URLEncodePath(Utility::buildRelativePath(navbookpath, navstylebookpath));
    QString texthref = Utility::URLEncodePath(Utility::buildRelativePath(navbookpath, first_section_bookpath));
    
    HTMLResource * html_resource = qobject_cast<HTMLResource *>(resource);
    SettingsStore ss;
    QString defaultLanguage = ss.defaultMetadataLang();
    QString navtitle = Landmarks::instance()->GetName("toc");
    QString guidetitle = Landmarks::instance()->GetName("landmarks");
    QString start = tr("Start");
    QString navtext = 
        EMPTY_NAV_FILE_START.arg(defaultLanguage).arg(defaultLanguage).arg(stylehref) +
        EMPTY_NAV_FILE_TOC.arg(navtitle).arg(texthref).arg(start) + 
        EMPTY_NAV_FILE_LANDMARKS.arg(guidetitle).arg(navtitle) +
        EMPTY_NAV_FILE_END;
    html_resource->SetText(navtext);
    html_resource->SaveToDisk();
    SetModified(true);
    return html_resource;
}


HTMLResource *Book::CreateEmptyHTMLFile(HTMLResource *resource, const QString &folderpath)
{
    HTMLResource *new_resource = CreateNewHTMLFile(folderpath);
    QString version = new_resource->GetEpubVersion();
    if (version.startsWith('2')) {
        new_resource->SetText(EMPTY_HTML_FILE);
    } else {
        new_resource->SetText(EMPTY_HTML5_FILE);
    }
    if (resource != NULL) {
        QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
        int reading_order = GetOPF()->GetReadingOrder(resource) + 1;

        if (reading_order > 0) {
            html_resources.insert(reading_order, new_resource);
            GetOPF()->UpdateSpineOrder(html_resources);
        }
    }

    SetModified(true);
    return new_resource;
}


void Book::MoveResourceAfter(HTMLResource *from_resource, HTMLResource *to_resource)
{
    if (from_resource == NULL || to_resource == NULL) {
        return;
    }

    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
    int to_after_reading_order = GetOPF()->GetReadingOrder(to_resource) + 1;
    int from_reading_order = GetOPF()->GetReadingOrder(from_resource) ;

    if (to_after_reading_order > 0) {
        html_resources.move(from_reading_order, to_after_reading_order);
        GetOPF()->UpdateSpineOrder(html_resources);
    }

    SetModified(true);
}

HTMLResource *Book::CreateHTMLCoverFile(QString text)
{
    HTMLResource *html_resource = CreateNewHTMLFile();
    QString version = html_resource->GetEpubVersion();
    html_resource->RenameTo(HTML_COVER_FILENAME);
    if (text.isEmpty()) {
      if (version.startsWith('2')) { 
            text = HTML_COVER_SOURCE;
        } else {
            text = HTML5_COVER_SOURCE;
        }
    }
    html_resource->SetText(text);
    html_resource->SaveToDisk();

    // Move file to start of book.
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
    html_resources.removeOne(html_resource);
    html_resources.prepend(html_resource);
    GetOPF()->UpdateSpineOrder(html_resources);

    SetModified(true);
    return html_resource;
}


CSSResource *Book::CreateHTMLTOCCSSFile()
{
    CSSResource *css_resource = CreateEmptyCSSFile();
    css_resource->SetText(SGC_TOC_CSS_FILE);
    css_resource->SaveToDisk();
    return css_resource;
}

CSSResource *Book::CreateIndexCSSFile()
{
    CSSResource *css_resource = CreateEmptyCSSFile();
    css_resource->SetText(SGC_INDEX_CSS_FILE);
    css_resource->SaveToDisk();
    return css_resource;
}

CSSResource *Book::CreateEmptyCSSFile(const QString &folderpath)
{
    TempFolder tempfolder;
    QString fullfilepath = tempfolder.GetPath() + "/" + m_Mainfolder->GetUniqueFilenameVersion(FIRST_CSS_NAME);
    Utility::WriteUnicodeTextFile("", fullfilepath);
    Resource * resource = m_Mainfolder->AddContentFileToFolder(fullfilepath,
							       true,
							       "text/css",
							       QString(),
							       folderpath);
    CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
    SetModified(true);
    return css_resource;
}


SVGResource *Book::CreateEmptySVGFile(const QString& folderpath)
{
    TempFolder tempfolder;
    QString fullfilepath = tempfolder.GetPath() + "/" + m_Mainfolder->GetUniqueFilenameVersion(FIRST_SVG_NAME);
    Utility::WriteUnicodeTextFile("", fullfilepath);
    Resource * resource = m_Mainfolder->AddContentFileToFolder(fullfilepath,
                                                               true,
                                                               "image/svg+xml",
                                                               QString(),
                                                               folderpath);
    SVGResource *svg_resource = qobject_cast<SVGResource *>(resource);
    SetModified(true);
    return svg_resource;
}


HTMLResource *Book::CreateSectionBreakOriginalResource(const QString &content, HTMLResource *originating_resource)
{
    if (originating_resource == NULL)
        return NULL;

    const QString originating_bookpath = originating_resource->GetRelativePath();

    // use this to get file extension for renaming purposes
    const QString originating_filename = originating_resource->Filename();

    // use this to get folder path for new resource
    const QString folder_path = Utility::startingDir(originating_bookpath);

    int reading_order = GetOPF()->GetReadingOrder(originating_resource);
    Q_ASSERT(reading_order >= 0);
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
    QString old_extension = originating_filename.right(originating_filename.length() - originating_filename.lastIndexOf("."));
    originating_resource->RenameTo(GetFirstUniqueSectionName(old_extension));
    HTMLResource *new_resource = CreateNewHTMLFile(folder_path);
    QString version = GetOPF()->GetEpubVersion();
    new_resource->RenameTo(originating_filename);
    new_resource->SetText(CleanSource::Mend(content, version));
    m_Mainfolder->SuspendWatchingResources();
    new_resource->SaveToDisk();
    m_Mainfolder->ResumeWatchingResources();
    html_resources.insert(reading_order, new_resource);
    GetOPF()->UpdateSpineOrder(html_resources);
    // Update references between the two new files. Since they used to be one single file we can
    // assume that each id is unique (if they aren't then the references were broken anyway).
    QList<HTMLResource *> new_files;
    new_files.append(originating_resource);
    new_files.append(new_resource);
    AnchorUpdates::UpdateAllAnchorsWithIDs(new_files);
    // Remove the original and new files from the list of html resources as we want to scan all
    // the other files for external references to the original file.
    html_resources.removeOne(originating_resource);
    html_resources.removeOne(new_resource);
    // Now, update references to the original file that are made in other files.
    // We can't assume that ids are unique in this case, and so need to use a different mechanism.
    AnchorUpdates::UpdateExternalAnchors(html_resources, originating_bookpath, new_files);
    // Update TOC entries as well if an NCX exists:
    NCXResource * ncx_resource = GetNCX();
    if (ncx_resource) {
        AnchorUpdates::UpdateTOCEntries(ncx_resource, originating_bookpath, new_files);
    }
    SetModified(true);
    return new_resource;
}


void Book::CreateNewSections(const QStringList &new_sections, HTMLResource *original_resource)
{
    const QString originating_bookpath = original_resource->GetRelativePath();
    int original_position = GetOPF()->GetReadingOrder(original_resource);
    Q_ASSERT(original_position >= 0);
    QString new_file_prefix = QFileInfo(original_resource->Filename()).baseName();
    QString file_extension = "." + QFileInfo(original_resource->Filename()).suffix();
    QString folder_path = Utility::startingDir(original_resource->GetRelativePath());

    if (new_sections.isEmpty()) {
        return;
    }

    TempFolder tempfolder;
    QFutureSynchronizer<NewSectionResult> sync;
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
    // A list of all the files that have not been involved in the split.
    // This will be used later when anchors are updated.
    QList<HTMLResource *> other_files = html_resources;
    other_files.removeOne(original_resource);
    int next_reading_order;

    if (original_position == -1) {
        next_reading_order = m_Mainfolder->GetHighestReadingOrder() + 1;
    } else {
        next_reading_order = original_position + 1;
    }

    for (int i = 0; i < new_sections.count(); ++i) {
        int reading_order = next_reading_order + i;
        NewSection sectionInfo;
        sectionInfo.source = new_sections.at(i);
        sectionInfo.reading_order = reading_order;
        sectionInfo.temp_folder_path = tempfolder.GetPath();
        sectionInfo.new_file_prefix = new_file_prefix;
        sectionInfo.file_suffix = i;
        sectionInfo.file_extension = file_extension;
	sectionInfo.folder_path = folder_path;
        sync.addFuture(
            QtConcurrent::run(
                this,
                &Book::CreateOneNewSection,
                sectionInfo));
    }

    sync.waitForFinished();
    QList<HTMLResource *> new_files;
    new_files.append(original_resource);
    QList<QFuture<NewSectionResult>> futures = sync.futures();

    if (original_position == -1) {
        // Add new sections to the end of the book
        for (int i = 0; i < futures.count(); ++i) {
            html_resources.append(futures.at(i).result().created_section);
            new_files.append(futures.at(i).result().created_section);
        }
    } else {
        // Insert the new files at the correct positions in the list
        // The new files need to be inserted in order from first to last
        for (int i = 0; i < new_sections.count(); ++i) {
            int reading_order = next_reading_order + i;

            if (futures.at(i).result().reading_order == reading_order) {
                html_resources.insert(reading_order , futures.at(i).result().created_section);
                new_files.append(futures.at(i).result().created_section);
            } else {
                // This is security code to protect against any mangling of the futures list by Qt
                for (int j = 0 ; j < futures.count() ; ++j) {
                    if (futures.at(j).result().reading_order == reading_order) {
                        html_resources.insert(reading_order , futures.at(j).result().created_section);
                        new_files.append(futures.at(j).result().created_section);
                        break;
                    }
                }
            }
        }
    }

    // Update anchor references between fragment ids in the new files. Since these all came from one single
    // file it's safe to assume that the fragment ids are all unique (since otherwise the references would be broken).
    AnchorUpdates::UpdateAllAnchorsWithIDs(new_files);
    // Now, update references to the original file that are made in other files.
    // We can't assume that ids are unique in this case, and so need to use a different mechanism.
    AnchorUpdates::UpdateExternalAnchors(other_files, original_resource->GetRelativePath(), new_files);
    // Update TOC entries as well if an NCX exists, they are optional on epub3
    NCXResource * ncx_resource = GetNCX();
    if (ncx_resource) {
        AnchorUpdates::UpdateTOCEntries(ncx_resource, originating_bookpath, new_files);
    }
    GetOPF()->UpdateSpineOrder(html_resources);
    SetModified(true);
}


bool Book::IsDataWellFormed(HTMLResource *html_resource)
{
    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(html_resource->GetText(), 
                                                                         html_resource->GetEpubVersion());
    return error.line == -1;
}


bool Book::IsDataOnDiskWellFormed(HTMLResource *html_resource)
{
    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(Utility::ReadUnicodeTextFile(html_resource->GetFullPath()), 
                                                                       html_resource->GetEpubVersion());
    return error.line == -1;
}

void Book::ReformatAllHTML(bool to_valid)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    SaveAllResourcesToDisk();
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
    bool book_modified = CleanSource::ReformatAll(html_resources, to_valid ? CleanSource::Mend : CleanSource::MendPrettify);
    if (book_modified) {
        SetModified();
    }
    QApplication::restoreOverrideCursor();
}


Resource *Book::PreviousResource(Resource *resource)
{
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
    int previous_file_reading_order = html_resources.indexOf(html_resource) - 1;

    if (previous_file_reading_order < 0) {
        previous_file_reading_order = 0;
    }

    HTMLResource *previous_html = html_resources[ previous_file_reading_order ];
    return qobject_cast<Resource *>(previous_html);
}

QHash <QString, QList<XhtmlDoc::XMLElement>> Book::GetLinkElements()
{
    QHash<QString, QList<XhtmlDoc::XMLElement>> links_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QList<XhtmlDoc::XMLElement>>> future = QtConcurrent::mapped(html_resources, GetLinkElementsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QList<XhtmlDoc::XMLElement> links;
        std::tie(bookpath, links) = future.resultAt(i);
        // Each target entry has a list of filenames that contain it
        links_in_html[bookpath] = links;
    }

    return links_in_html;
}

std::tuple<QString, QList<XhtmlDoc::XMLElement>> Book::GetLinkElementsInHTMLFileMapped(HTMLResource *html_resource)
{
    return std::make_tuple(html_resource->GetRelativePath(),
                      XhtmlDoc::GetTagsInDocument(html_resource->GetText(), "a"));
}

QStringList Book::GetStyleUrlsInHTMLFiles()
{
    QStringList styles_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetStyleUrlsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QStringList style_bookpaths;
        std::tie(bookpath, style_bookpaths) = future.resultAt(i);
        // Each target entry has a list of filenames that contain it
        styles_in_html.append(style_bookpaths);
    }
    return styles_in_html;
}

std::tuple<QString, QStringList> Book::GetStyleUrlsInHTMLFileMapped(HTMLResource *html_resource)
{
    QString html_bookpath = html_resource->GetRelativePath();
    QString startdir = html_resource->GetFolder();
    // we need to convert this hreflist to bookpaths if possible
    QStringList urllist = XhtmlDoc::GetAllDescendantStyleUrls(html_resource->GetText());
    QStringList bookpaths;
    QRegularExpression url_file_search("url\\s*\\(\\s*['\"]?([^\\(\\)'\"]*)[\"']?\\)");
    foreach (QString url, urllist) {
        QRegularExpressionMatch match = url_file_search.match(url);
        if (match.hasMatch()) {
	    QString ahref = match.captured(1);
            if (ahref.indexOf(":") == -1) {
	        bookpaths << Utility::buildBookPath(ahref, startdir);
            }
        }
    }
    return std::make_tuple(html_bookpath, bookpaths);
}

QHash<QString, QStringList> Book::GetIdsInHTMLFiles()
{
    QHash<QString, QStringList> ids_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetIdsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QStringList ids;
        std::tie(bookpath, ids) = future.resultAt(i);
        // Each target entry has a list of filenames that contain it
        ids_in_html[bookpath] = ids;
    }

    return ids_in_html;
}

std::tuple<QString, QStringList> Book::GetIdsInHTMLFileMapped(HTMLResource *html_resource)
{
    return std::make_tuple(html_resource->GetRelativePath(),
                           XhtmlDoc::GetAllDescendantIDs(html_resource->GetText()));
}

QStringList Book::GetIdsInHTMLFile(HTMLResource *html_resource)
{
    return XhtmlDoc::GetAllDescendantIDs(html_resource->GetText());
}


QStringList Book::GetIdsInHrefs()
{
    QStringList ids;
    QHash<QString, QStringList> hrefs = GetHrefsInHTMLFiles();
    QHashIterator<QString, QStringList> it(hrefs);

    while (it.hasNext()) {
        it.next();
        foreach(QString href, it.value()) {
            if (href.contains('#')) {
                href = href.right(href.length() - href.indexOf("#") - 1);

                if (!href.isEmpty() && !ids.contains(href)) {
                    ids.append(href);
                }
            }
        }
    }

    // Return the list of ids used in hrefs in the book
    return ids;
}


QHash<QString, QStringList> Book::GetHrefsInHTMLFiles()
{
    QHash<QString, QStringList> hrefs_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetHrefsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QStringList hrefs;
        std::tie(bookpath, hrefs) = future.resultAt(i);
        // Each target entry has a list of filenames that contain it
        hrefs_in_html[bookpath] = hrefs;
    }

    return hrefs_in_html;
}

std::tuple<QString, QStringList> Book::GetHrefsInHTMLFileMapped(HTMLResource *html_resource)
{
    return std::make_tuple(html_resource->GetRelativePath(),
                           XhtmlDoc::GetAllDescendantHrefs(html_resource->GetText()));
}

QStringList Book::GetClassesInHTMLFile(HTMLResource *html_resource)
{
    return XhtmlDoc::GetAllDescendantClasses(html_resource->GetText());
}

QHash<QString, QStringList> Book::GetImagesInHTMLFiles()
{
    QHash<QString, QStringList> images_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetImagesInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QStringList image_files;
        std::tie(bookpath, image_files) = future.resultAt(i);
        images_in_html[bookpath] = image_files;
    }
    return images_in_html;
}

QHash< QString, std::pair<int,int> > Book::GetSpellWordCountsInHTMLFiles()
{
    QHash< QString, std::pair<int,int> > words_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, std::pair<int,int> > > future = QtConcurrent::mapped(html_resources, GetWordCountsInHTMLFileMapped);
    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
	std::pair<int, int> word_counts;
        std::tie(bookpath, word_counts) = future.resultAt(i);
        words_in_html[bookpath] = word_counts;
    }
    return words_in_html;
}


std::tuple<QString, std::pair<int,int> > Book::GetWordCountsInHTMLFileMapped(HTMLResource *html_resource)
{
    QString html_bookpath = html_resource->GetRelativePath();
    std::pair<int,int> counts;
    counts.first = HTMLSpellCheck::CountAllWords(html_resource->GetText());
    counts.second = HTMLSpellCheck::CountMisspelledWords(html_resource->GetText());
    return std::make_tuple(html_bookpath, counts);
}


QHash<QString, QStringList> Book::GetVideoInHTMLFiles()
{
    QHash<QString, QStringList> video_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetVideoInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QStringList video_files;
        std::tie(bookpath, video_files) = future.resultAt(i);
        video_in_html[bookpath] = video_files;
    }
    return video_in_html;
}

QHash<QString, QStringList> Book::GetAudioInHTMLFiles()
{
    QHash<QString, QStringList> audio_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetAudioInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QStringList audio_files;
        std::tie(bookpath, audio_files) = future.resultAt(i);
        audio_in_html[bookpath] = audio_files;
    }
    return audio_in_html;
}

QHash<QString, QStringList> Book::GetHTMLFilesUsingMedia()
{
    QHash<QString, QStringList> html_files;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);

    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetMediaInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString html_bookpath;
        QStringList media_bookpaths;
        std::tie(html_bookpath, media_bookpaths) = future.resultAt(i);
        foreach(QString media_bookpath, media_bookpaths) {
            html_files[media_bookpath].append(html_bookpath);
        }
    }

    return html_files;
}

QHash<QString, QStringList> Book::GetHTMLFilesUsingImages()
{
    QHash<QString, QStringList> html_files;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);

    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetImagesInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString html_bookpath;
        QStringList image_bookpaths;
        std::tie(html_bookpath, image_bookpaths) = future.resultAt(i);
        foreach(QString bookpath, image_bookpaths) {
            Resource * resource = m_Mainfolder->GetResourceByBookPath(html_bookpath);
            html_files[bookpath].append(resource->ShortPathName());
        }
    }

    return html_files;
}

std::tuple<QString, QStringList> Book::GetMediaInHTMLFileMapped(HTMLResource *html_resource)
{
    QString html_bookpath = html_resource->GetRelativePath();
    QString startdir = html_resource->GetFolder();
    QStringList media_hrefs = XhtmlDoc::GetAllMediaPathsFromMediaChildren(html_resource->GetText(), 
                                                                       GIMAGE_TAGS + GVIDEO_TAGS + GAUDIO_TAGS);
    QStringList media_bookpaths;
    foreach(QString ahref, media_hrefs) {
        if (ahref.indexOf(":") == -1) {
            media_bookpaths << Utility::buildBookPath(ahref, startdir);
	}
    }
    return std::make_tuple(html_bookpath, media_bookpaths);
}

std::tuple<QString, QStringList> Book::GetImagesInHTMLFileMapped(HTMLResource *html_resource)
{
    QString html_bookpath = html_resource->GetRelativePath();
    QString startdir = html_resource->GetFolder();
    QStringList image_hrefs = XhtmlDoc::GetAllMediaPathsFromMediaChildren(html_resource->GetText(), GIMAGE_TAGS);
    QStringList image_bookpaths;
    foreach(QString ahref, image_hrefs) {
        if (ahref.indexOf(":") == -1) {
	    image_bookpaths << Utility::buildBookPath(ahref, startdir);
        }
    }
    return std::make_tuple(html_bookpath, image_bookpaths);
}

std::tuple<QString, QStringList> Book::GetVideoInHTMLFileMapped(HTMLResource *html_resource)
{
    QString html_bookpath = html_resource->GetRelativePath();
    QString startdir = html_resource->GetFolder();
    QStringList video_hrefs = XhtmlDoc::GetAllMediaPathsFromMediaChildren(html_resource->GetText(), GVIDEO_TAGS);
    QStringList video_bookpaths;
    foreach(QString ahref, video_hrefs) {
        if (ahref.indexOf(":") == -1) {
	    video_bookpaths << Utility::buildBookPath(ahref, startdir);
        }
    }
    return std::make_tuple(html_bookpath, video_bookpaths);
}

std::tuple<QString, QStringList> Book::GetAudioInHTMLFileMapped(HTMLResource *html_resource)
{
    QString html_bookpath = html_resource->GetRelativePath();
    QString startdir = html_resource->GetFolder();
    QStringList audio_hrefs = XhtmlDoc::GetAllMediaPathsFromMediaChildren(html_resource->GetText(), GAUDIO_TAGS);
    QStringList audio_bookpaths;
    foreach(QString ahref, audio_hrefs) {
        if (ahref.indexOf(":") == -1) {
	    audio_bookpaths << Utility::buildBookPath(ahref, startdir);
        }
    }
    return std::make_tuple(html_bookpath, audio_bookpaths);
}

QList<HTMLResource *> Book::GetNonWellFormedHTMLFiles()
{
    QList<HTMLResource *> malformed_resources;
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture< std::pair<HTMLResource*, bool> > well_future;
    well_future = QtConcurrent::mapped(html_resources, ResourceWellFormedMap);
    for (int i = 0; i < well_future.results().count(); i++) {
	std::pair<HTMLResource*, bool> res = well_future.resultAt(i);
        if (!res.second) malformed_resources << res.first;
    }
    return malformed_resources;
}

std::pair<HTMLResource*, bool> Book::ResourceWellFormedMap(HTMLResource * html_resource) {
    std::pair<HTMLResource*, bool> res;
    res.first = html_resource;
    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(html_resource->GetText(),
                                                                         html_resource->GetEpubVersion());
    res.second = (error.line == -1);
    return res;
}

QSet<QString> Book::GetWordsInHTMLFiles()
{
    QStringList all_words;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<QStringList> future = QtConcurrent::mapped(html_resources, GetWordsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QStringList result = future.resultAt(i);
        all_words.append(result);
    }

    return all_words.toSet();
}

QStringList Book::GetWordsInHTMLFileMapped(HTMLResource *html_resource)
{
    return HTMLSpellCheck::GetAllWords(html_resource->GetText());
}

QHash<QString, int> Book::GetUniqueWordsInHTMLFiles()
{
    QHash<QString, int> all_words;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<QStringList> future = QtConcurrent::mapped(html_resources, GetWordsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QStringList result = future.resultAt(i);
        foreach (QString word, result) {
            if (all_words.contains(word)) {
                all_words[word]++;
            } else {
                all_words[word] = 1;
            }
        }
    }

    return all_words;
}

QHash<QString, QStringList> Book::GetStylesheetsInHTMLFiles()
{
    QHash<QString, QStringList> links_in_html;
    const QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(false);
    QFuture<std::tuple<QString, QStringList>> future = QtConcurrent::mapped(html_resources, GetStylesheetsInHTMLFileMapped);

    for (int i = 0; i < future.results().count(); i++) {
        QString bookpath;
        QStringList links;
        std::tie(bookpath, links) = future.resultAt(i);
        links_in_html[bookpath] = links;
    }
    return links_in_html;
}

std::tuple<QString, QStringList> Book::GetStylesheetsInHTMLFileMapped(HTMLResource *html_resource)
{
    QString html_bookpath = html_resource->GetRelativePath();
    QString startdir = html_resource->GetFolder();
    QStringList link_hrefs = XhtmlDoc::GetLinkedStylesheets(html_resource->GetText());
    QStringList link_bookpaths;
    foreach(QString ahref, link_hrefs) {
        if (ahref.indexOf(":") == -1) {
            link_bookpaths << Utility::buildBookPath(ahref, startdir);
        }
    }
    return std::make_tuple(html_bookpath, link_bookpaths);
}

QStringList Book::GetStylesheetsInHTMLFile(HTMLResource *html_resource)
{
    // convert links relative to a html resource to their book paths
    QStringList stylelinks = XhtmlDoc::GetLinkedStylesheets(html_resource->GetText());
    QStringList results;
    QString html_folder = html_resource->GetFolder();
    foreach(QString stylelink, stylelinks) {
       results.append(Utility::buildBookPath(stylelink, html_folder));
    }
    return results;
}

Resource *Book::MergeResources(QList<Resource *> resources)
{

    // Make sure that the nav resource is not part of a merge
    Resource* nav_resource = GetConstOPF()->GetNavResource();
    if (nav_resource && resources.contains(nav_resource)) {
        // return the nav_resource as the failed resource
        return nav_resource;
    }

    QProgressDialog progress(QObject::tr("Merging Files.."), 0, 0, resources.count(), QApplication::activeWindow());
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;
    Resource *sink_resource = resources.takeFirst();
    HTMLResource *sink_html_resource = qobject_cast<HTMLResource *>(sink_resource);

    if (!IsDataWellFormed(sink_html_resource)) {
        return sink_resource;
    }

    QStringList new_bodies;
    QList<QString> merged_bookpaths;
    QString version = sink_html_resource->GetEpubVersion();
    {
        GumboInterface gi = GumboInterface(sink_html_resource->GetText(), version);
        new_bodies << gi.get_body_contents();
        Resource *failed_resource = NULL;
        
        // Now iterate across all the other resources merging them into this resource
        foreach(Resource *source_resource, resources) {

            // Set progress value and ensure dialog has time to display when doing extensive updates
            progress.setValue(progress_value++);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

            HTMLResource *source_html_resource = qobject_cast<HTMLResource *>(source_resource);
            if (!IsDataWellFormed(source_html_resource)) {
                failed_resource = source_resource;
                break;
            }

            // Get the html document for this source resource.
            GumboInterface ngi = GumboInterface(source_html_resource->GetText(), version);
            new_bodies << ngi.get_body_contents();
            merged_bookpaths.append(source_resource->GetRelativePath());
        }

        if (failed_resource != NULL) {
            // Abort the merge process. We will discard whatever we had merged so far
            return failed_resource;
        }
        QString new_body = new_bodies.join("");
        QString new_source = gi.perform_body_updates(new_body);
        // Now all fragments have been merged into this sink document, serialize and store it.
        sink_html_resource->SetText(new_source);
        // Now safe to do the delete
        foreach(Resource *source_resource, resources) {
            // Need to alert FolderKeeper that these are going away to properly update its
            // m_Resources hash to prevent stale values from deleted resources hanging around
            m_Mainfolder->RemoveResource(source_resource);
            source_resource->Delete();
        }
    }
    progress.setValue(progress_value++);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    // It is the user's responsibility to ensure that all ids used across the two merged files are unique.
    // Reconcile all references to the files that were merged.
    QList<HTMLResource *> html_resources = m_Mainfolder->GetResourceTypeList<HTMLResource>(true);
    AnchorUpdates::UpdateAllAnchors(html_resources, merged_bookpaths, sink_html_resource);
    NCXResource * ncx_resource = GetNCX();
    if (ncx_resource) {
        AnchorUpdates::UpdateTOCEntriesAfterMerge(ncx_resource, 
						  sink_html_resource->GetRelativePath(),
						  merged_bookpaths);
    }
    SetModified(true);
    return NULL;
}

QList <Resource *> Book::GetAllResources()
{
    return m_Mainfolder->GetResourceList();
}

void Book::SaveAllResourcesToDisk()
{
    QList<Resource *> resources = m_Mainfolder->GetResourceList();
    m_Mainfolder->SuspendWatchingResources();
    QtConcurrent::blockingMap(resources, SaveOneResourceToDisk);
    m_Mainfolder->ResumeWatchingResources();
}


bool Book::IsModified() const
{
    return m_IsModified;
}


bool Book::HasObfuscatedFonts() const
{
    QList<FontResource *> font_resources = m_Mainfolder->GetResourceTypeList<FontResource>();

    if (font_resources.empty()) {
        return false;
    }

    foreach(FontResource *font_resource, font_resources) {
        if (!font_resource->GetObfuscationAlgorithm().isEmpty()) {
            return true;
        }
    }
    return false;
}

void Book::ResourceUpdatedFromDisk(Resource *resource)
{
    SetModified(true);
    emit ResourceUpdatedFromDiskRequest(resource);
}

void Book::SetModified(bool modified)
{
    bool old_modified_state = m_IsModified;
    m_IsModified = modified;

    if (modified != old_modified_state) {
        emit ModifiedStateChanged(m_IsModified);
    }
}

std::tuple<bool, QString, QString> Book::HasUndefinedURLFragments()
{
    QList<HTMLResource *> html_resources = GetHTMLResources();
    bool hasUndefinedUrlFrags = false;

    QStringList html_bookpaths;
    foreach(HTMLResource *html_resource, html_resources) {
        html_bookpaths.append(html_resource->GetRelativePath());
    }

    // The key to both here is now a bookpath
    QHash<QString, QStringList> links = GetRelLinksInAllFiles(html_resources);
    QHash<QString, QStringList> all_ids = GetIDsInAllFiles(html_resources);

    foreach(HTMLResource *html_resource, html_resources) {
        QString bookpath = html_resource->GetRelativePath();
	QString htmldir = html_resource->GetFolder();
        foreach(QString ahref, links[bookpath]) {
	    // each links is relative to the bookpath resource and already unquoted
            // convert this href to a bookpath href
	    std::pair<QString,QString> hrefparts = Utility::parseHREF(ahref);
	    QString attpath = hrefparts.first;
	    QString dest_bookpath;
	    if (attpath.isEmpty()) {
	        dest_bookpath = bookpath;
	    } else {
	        dest_bookpath = Utility::buildBookPath(attpath, htmldir);
	    }
	    QString dest_id = hrefparts.second;
	    if (dest_id.startsWith("#")) dest_id = dest_id.mid(1,-1);

	    if (!dest_id.isEmpty()) {
	        if (html_bookpaths.contains(dest_bookpath) && !all_ids[dest_bookpath].contains(dest_id)) {
	            qDebug() << "huh: " << bookpath << dest_bookpath << dest_id << hrefparts.second;
		    return std::make_tuple(true, ahref, bookpath);
	        }
	    }
        }
    }
    return std::make_tuple(false, QString(), QString());
}

QHash<QString, QStringList> Book::GetRelLinksInAllFiles(const QList<HTMLResource *> &html_resources)
{
    const QList<QPair<QString, QStringList>> &links_in_files = QtConcurrent::blockingMapped(html_resources, GetRelLinksInOneFile);
    QHash<QString, QStringList> links_in_files_map;
    for (int i = 0; i < links_in_files.count(); ++i) {
        QPair<QString, QStringList> entry = links_in_files.at(i);
        links_in_files_map.insert(entry.first, entry.second);
    }
    return links_in_files_map;
}


QHash<QString, QStringList> Book::GetIDsInAllFiles(const QList<HTMLResource *> &html_resources)
{
    const QList<QPair<QString, QStringList>> &IDs_in_files = QtConcurrent::blockingMapped(html_resources, GetOneFileIDs);
    QHash<QString, QStringList> ID_map;

    for (int i = 0; i < IDs_in_files.count(); ++i) {
        QPair<QString, QStringList> entry = IDs_in_files.at(i);
        ID_map.insert(entry.first, entry.second);
    }
    return ID_map;
}

void Book::SaveOneResourceToDisk(Resource *resource)
{
    resource->SaveToDisk(true);
}


Book::NewSectionResult Book::CreateOneNewSection(NewSection section_info)
{
    return CreateOneNewSection(section_info, QHash<QString, QString>());
}


Book::NewSectionResult Book::CreateOneNewSection(NewSection section_info,
        const QHash<QString, QString> &html_updates)
{
    QString filename = section_info.new_file_prefix % "_" % QString("%1").arg(section_info.file_suffix + 1, 4, 10, QChar('0')) + section_info.file_extension;
    QString fullfilepath = section_info.temp_folder_path + "/" + filename;
    Utility::WriteUnicodeTextFile("PLACEHOLDER", fullfilepath);
    Resource * resource = m_Mainfolder->AddContentFileToFolder(fullfilepath, 
							       true, 
							       QString(), 
							       QString(), 
							       section_info.folder_path);
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
    Q_ASSERT(html_resource);
    QString version = html_resource->GetEpubVersion();

    if (html_updates.isEmpty()) {
        html_resource->SetText(CleanSource::Mend(section_info.source, version));
    } else {
        QString currentpath = html_resource->GetCurrentBookRelPath();
	QString newbookpath = html_resource->GetRelativePath();
        html_resource->SetText(PerformHTMLUpdates(CleanSource::Mend(section_info.source, version),
					     newbookpath,
                                             html_updates, QHash<QString, QString>(), 
                                             currentpath, version)() );
        html_resource->SetCurrentBookRelPath("");
    }

    NewSectionResult section;
    section.created_section = html_resource;
    section.reading_order = section_info.reading_order;
    return section;
}

// Links hrefs are unquoted but otherwise untouched 
QPair<QString, QStringList> Book::GetRelLinksInOneFile(HTMLResource *html_resource)
{
    Q_ASSERT(html_resource);
    QReadLocker locker(&html_resource->GetLock());
    QString htmldir = html_resource->GetFolder();
    GumboInterface gi = GumboInterface(html_resource->GetText(), html_resource->GetEpubVersion());
    gi.parse();
    QPair<QString, QStringList> link_pair;
    QStringList hreflist;
    const QList<GumboNode*> anchor_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_A);
    for (int i = 0; i < anchor_nodes.length(); ++i) {
        GumboNode* node = anchor_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");
        // We find the hrefs that are relative and contain an href.
        if (attr && QUrl(QString::fromUtf8(attr->value)).isRelative()) {
	    QString attpath = Utility::URLDecodePath(QString::fromStdString(attr->value));
            hreflist.append(attpath);
        }
    }
    link_pair.first = html_resource->GetRelativePath();
    link_pair.second = hreflist;
    return link_pair;
}


QPair<QString, QStringList> Book::GetOneFileIDs(HTMLResource *html_resource)
{
    Q_ASSERT(html_resource);
    QReadLocker locker(&html_resource->GetLock());
    QString newsource = html_resource->GetText();
    QString version = html_resource->GetEpubVersion();
    GumboInterface gi = GumboInterface(newsource, version);
    gi.parse();
    QPair<QString, QStringList> id_pair;
    QStringList ids = gi.get_all_values_for_attribute(QString("id"));
    id_pair.first = html_resource->GetRelativePath();
    id_pair.second = ids;
    return id_pair;
}
