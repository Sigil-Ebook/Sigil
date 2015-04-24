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

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QObject>

#include "BookManipulation/CleanSource.h"
#include "Exporters/NCXWriter.h"
#include "ResourceObjects/NCXResource.h"
#include "sigil_constants.h"

static const QString TEMPLATE_TEXT =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE ncx PUBLIC \"-//NISO//DTD ncx 2005-1//EN\"\n"
    "   \"http://www.daisy.org/z3986/2005/ncx-2005-1.dtd\">\n"
    "<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n"
    "<head>\n"
    "   <meta name=\"dtb:uid\" content=\"ID_UNKNOWN\" />\n"
    "   <meta name=\"dtb:depth\" content=\"0\" />\n"
    "   <meta name=\"dtb:totalPageCount\" content=\"0\" />\n"
    "   <meta name=\"dtb:maxPageNumber\" content=\"0\" />\n"
    "</head>\n"
    "<docTitle>\n"
    "   <text>Unknown</text>\n"
    "</docTitle>\n"
    "<navMap>\n"
    "<navPoint id=\"navPoint-1\" playOrder=\"1\">\n"
    "  <navLabel>\n"
    "    <text>%1</text>\n"
    "  </navLabel>\n"
    "  <content src=\"Text/%2\" />\n"
    "</navPoint>\n"
    "</navMap>\n"
    "</ncx>";


NCXResource::NCXResource(const QString &mainfolder, const QString &fullfilepath, QObject *parent)
    : XMLResource(mainfolder, fullfilepath, parent)
{
    FillWithDefaultText();
    // Make sure the file exists on disk.
    // Among many reasons, this also solves the problem
    // with the Book Browser not displaying an icon for this resource.
    SaveToDisk();
}


bool NCXResource::RenameTo(const QString &new_filename)
{
    // The user is not allowed to rename the NCX file.
    return false;
}


Resource::ResourceType NCXResource::Type() const
{
    return Resource::NCXResourceType;
}


void NCXResource::SetMainID(const QString &main_id)
{
    SetText(GetText().replace("ID_UNKNOWN", main_id));
}


bool NCXResource::GenerateNCXFromBookContents(const Book *book)
{
    bool is_changed = false;
    QByteArray raw_ncx;
    QBuffer buffer(&raw_ncx);
    buffer.open(QIODevice::WriteOnly);
    NCXWriter ncx(book, buffer);
    ncx.WriteXMLFromHeadings();
    buffer.close();
    QString new_text = CleanSource::ProcessXML(QString::fromUtf8(raw_ncx.constData(), raw_ncx.size()));
    QString existing_text = GetText();

    // Only update the resource if have changed. Note that this is_changed trick will not
    // work after first loading an EPUB, because the metadata elements have their attributes
    // in swapped in a different order from the xhtml processing. 
    if (new_text != existing_text) {
        SetText(new_text);
        is_changed = true;
    }

    return is_changed;
}


void NCXResource::GenerateNCXFromTOCContents(const Book *book, NCXModel *ncx_model)
{
    //    QByteArray raw_ncx;
    //    QBuffer buffer(&raw_ncx);
    //    buffer.open(QIODevice::WriteOnly);
    //    NCXWriter ncx(book, buffer, ncx_model.GetRootNCXEntry());
    //    ncx.WriteXML();
    //    buffer.close();
    //    SetText(CleanSource::ProcessXML(QString::fromUtf8(raw_ncx.constData(), raw_ncx.size())));
    GenerateNCXFromTOCEntries(book, ncx_model->GetRootNCXEntry());
}

void NCXResource::GenerateNCXFromTOCEntries(const Book *book, NCXModel::NCXEntry ncx_root_entry)
{
    QByteArray raw_ncx;
    QBuffer buffer(&raw_ncx);
    buffer.open(QIODevice::WriteOnly);
    NCXWriter ncx(book, buffer, ncx_root_entry);
    ncx.WriteXML();
    buffer.close();
    SetText(CleanSource::ProcessXML(QString::fromUtf8(raw_ncx.constData(), raw_ncx.size())));
}


void NCXResource::FillWithDefaultText()
{
    SetText(TEMPLATE_TEXT.arg(tr("Start")).arg(FIRST_SECTION_NAME));
}


QString NCXResource::GetRelativePathToRoot() const
{
    QFileInfo info(GetFullPath());
    QDir parent_dir = info.dir();
    QString parent_name = parent_dir.dirName();
    return parent_name + "/" + Filename();
}
