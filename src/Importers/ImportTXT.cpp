/************************************************************************
**
**  Copyright (C) 2015-2020 Kevin B. Hendricks, Stratford, Ontario Canada
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

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Importers/ImportTXT.h"
#include "Misc/SettingsStore.h"
#include "Misc/TempFolder.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

const QString FIRST_SECTION_PREFIX = "Section0001";
const QString FIRST_SECTION_NAME   = FIRST_SECTION_PREFIX + ".xhtml";

// Constructor;
// The parameter is the file to be imported
ImportTXT::ImportTXT(const QString &fullfilepath)
    : Importer(fullfilepath)
{
    SettingsStore ss;
    m_EpubVersion = ss.defaultVersion(); 
}


// Reads and parses the file
// and returns the created Book
QSharedPointer<Book> ImportTXT::GetBook(bool extract_metadata)
{
    
    if (!Utility::IsFileReadable(m_FullFilePath)) {
        throw(CannotReadFile(m_FullFilePath.toStdString()));
    }

    // First handle any new created Books by making sure there is 
    // an OPF in the current Book
    if (!m_Book->GetConstOPF()) {
        m_Book->GetFolderKeeper()->AddOPFToFolder(m_EpubVersion);
    } 

    QString source = LoadSource();
    HTMLResource * new_resource = CreateHTMLResource(source);
    InitializeHTMLResource(source, new_resource);

    // Before returning the new book, if it is epub3, make sure it has a nav
    if (m_EpubVersion.startsWith('3')) {
        HTMLResource* nav_resource = m_Book->GetConstOPF()->GetNavResource();
        if (!nav_resource) {
            HTMLResource * nav_resource = m_Book->CreateEmptyNavFile(true);
            m_Book->GetOPF()->SetNavResource(nav_resource);
            m_Book->GetOPF()->SetItemRefLinear(nav_resource, false);
        }
    }

    // Before returning the new book, if it is epub2, make sure it has an ncx
    if (m_EpubVersion.startsWith('2')) {
        NCXResource* ncx_resource = m_Book->GetNCX();
        if (!ncx_resource) {
            // add NCX using "toc.ncx" with id "ncx" right beside the opf
            QString ncxbookpath = Utility::startingDir(m_Book->GetOPF()->GetRelativePath()) + "/toc.ncx";
            ncx_resource = m_Book->GetFolderKeeper()->AddNCXToFolder(m_EpubVersion, ncxbookpath);
            QString NCXId = m_Book->GetOPF()->AddNCXItem(ncx_resource->GetFullPath(),"ncx");
            m_Book->GetOPF()->UpdateNCXOnSpine(NCXId);
            QString first_xhtml_bookpath = new_resource->GetRelativePath();
            ncx_resource->FillWithDefaultTextToBookPath(m_EpubVersion, first_xhtml_bookpath);
        }
    }
    return m_Book;
}


QString ImportTXT::LoadSource() const
{
    QString source = Utility::ReadUnicodeTextFile(m_FullFilePath);
    source = CreateParagraphs(source.split(QChar::ParagraphSeparator));
    return CleanSource::Mend(source, m_EpubVersion);
}


HTMLResource *ImportTXT::CreateHTMLResource(const QString &source)
{
    TempFolder tempfolder;
    QString fullfilepath = tempfolder.GetPath() + "/" + FIRST_SECTION_NAME;
    Utility::WriteUnicodeTextFile(source, fullfilepath);
    m_Book->GetFolderKeeper()->AddContentFileToFolder(fullfilepath);
    return m_Book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>()[ 0 ];
}


void ImportTXT::InitializeHTMLResource(const QString &source, HTMLResource *resource)
{
    resource->SetText(source);
}


// Accepts a list of text lines and returns
// a string with paragraphs wrapped into <p> tags
QString ImportTXT::CreateParagraphs(const QStringList &lines) const
{
    QString text = "";
    QString paragraph = "<p>";
    int num_lines = lines.count();

    for (int i = 0; i < num_lines; ++i) {
        QString line = lines.at(i);

        if (line.isEmpty() || line[ 0 ].isSpace()) {
            text.append(paragraph.append("</p>\n"));
            paragraph = "<p>";
        }

        // We prepend a space so words on
        // line breaks don't get merged
        paragraph.append(QString(line.prepend(" ")).toHtmlEscaped());
    }

    text.append(paragraph.append("</p>\n"));
    return text;
}


