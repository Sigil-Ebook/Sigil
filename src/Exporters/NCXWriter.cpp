/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks, Stratford, Ontario, Canada
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

#include <QtCore/QXmlStreamWriter>

#include "BookManipulation/Book.h"
#include "Exporters/NCXWriter.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_constants.h"

NCXWriter::NCXWriter(const Book *book, QIODevice &device)
    :
    XMLWriter(book, device),
    m_Headings(Headings::MakeHeadingHeirarchy(
                 Headings::GetHeadingList(book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>(true)))),
    m_TOCRootEntry(TOCModel::TOCEntry()),
    m_version(book->GetConstOPF()->GetEpubVersion())
    
{
}


NCXWriter::NCXWriter(const Book *book, QIODevice &device, TOCModel::TOCEntry toc_root_entry)
    :
    XMLWriter(book, device),
    m_TOCRootEntry(toc_root_entry),
    m_version(book->GetConstOPF()->GetEpubVersion())

{
}


void NCXWriter::WriteXMLFromHeadings()
{
    m_TOCRootEntry = ConvertHeadingsToTOC();
    WriteXML();
}


void NCXWriter::WriteXML()
{
    m_Writer->writeStartDocument();
    if (m_version.startsWith('2')) {
        m_Writer->writeDTD("<!DOCTYPE ncx PUBLIC \"-//NISO//DTD ncx 2005-1//EN\"\n"
                           "   \"http://www.daisy.org/z3986/2005/ncx-2005-1.dtd\">\n");
    }
    m_Writer->writeStartElement("ncx");
    m_Writer->writeAttribute("xmlns", "http://www.daisy.org/z3986/2005/ncx/");
    m_Writer->writeAttribute("version", "2005-1");
    WriteHead();
    WriteDocTitle();
    WriteNavMap();
    m_Writer->writeEndElement();
    m_Writer->writeEndDocument();
}


void NCXWriter::WriteHead()
{
    m_Writer->writeStartElement("head");
    m_Writer->writeEmptyElement("meta");
    m_Writer->writeAttribute("name", "dtb:uid");
    m_Writer->writeAttribute("content", m_Book->GetPublicationIdentifier());
    m_Writer->writeEmptyElement("meta");
    m_Writer->writeAttribute("name", "dtb:depth");
    m_Writer->writeAttribute("content", QString::number(GetTOCDepth()));
    m_Writer->writeEmptyElement("meta");
    m_Writer->writeAttribute("name", "dtb:totalPageCount");
    m_Writer->writeAttribute("content", "0");
    m_Writer->writeEmptyElement("meta");
    m_Writer->writeAttribute("name", "dtb:maxPageNumber");
    m_Writer->writeAttribute("content", "0");
    m_Writer->writeEndElement();
}


void NCXWriter::WriteDocTitle()
{
    QString document_title;
    QList<QVariant> titles = m_Book->GetMetadataValues("dc:title");

    if (titles.isEmpty()) {
        document_title = "Unknown";
    } else { // FIXME: handle multiple titles
        document_title = titles.first().toString();
    }

    m_Writer->writeStartElement("docTitle");
    m_Writer->writeTextElement("text", document_title);
    m_Writer->writeEndElement();
}


void NCXWriter::WriteNavMap()
{
    int play_order = 1;
    m_Writer->writeStartElement("navMap");

    if (!m_TOCRootEntry.children.isEmpty()) {
        // The NavMap is written recursively;
        // WriteNavPoint is called for each entry in the tree
        foreach(TOCModel::TOCEntry entry, m_TOCRootEntry.children) {
            WriteNavPoint(entry, play_order);
        }
    } else {
        // No headings? Well the spec *demands* an NCX file
        // with a NavMap with at least one NavPoint, so we
        // write a dummy one.
        WriteFallbackNavPoint();
    }

    m_Writer->writeEndElement();
}


void NCXWriter::WriteFallbackNavPoint()
{
    m_Writer->writeStartElement("navPoint");
    m_Writer->writeAttribute("id", QString("navPoint-%1").arg(1));
    m_Writer->writeAttribute("playOrder", QString("%1").arg(1));
    m_Writer->writeStartElement("navLabel");
    m_Writer->writeTextElement("text", "Start");
    m_Writer->writeEndElement();
    QList<HTMLResource *> html_resources = m_Book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>(true);
    Q_ASSERT(!html_resources.isEmpty());
    m_Writer->writeEmptyElement("content");
    m_Writer->writeAttribute("src", Utility::URLEncodePath(html_resources.at(0)->GetRelativePathToOEBPS()));
    m_Writer->writeEndElement();
}


TOCModel::TOCEntry NCXWriter::ConvertHeadingsToTOC()
{
    TOCModel::TOCEntry toc_root;
    foreach(const Headings::Heading & heading, m_Headings) {
        toc_root.children.append(ConvertHeadingWalker(heading));
    }
    return toc_root;
}


TOCModel::TOCEntry NCXWriter::ConvertHeadingWalker(const Headings::Heading &heading)
{
    TOCModel::TOCEntry toc_child;

    if (heading.include_in_toc) {
        toc_child.text = heading.text;
        QString heading_file = heading.resource_file->GetRelativePathToOEBPS();
        QString id_to_use = heading.id;

        // If this heading appears right after a section break,
        // then it "represents" and links to its file; otherwise,
        // we link to the heading element directly
        if (heading.at_file_start) {
            toc_child.target = Utility::URLEncodePath(heading_file);
        } else {
            QString path = heading_file + "#" + id_to_use;
            toc_child.target = Utility::URLEncodePath(path);
        }
    }

    foreach(Headings::Heading child_heading, heading.children) {
        toc_child.children.append(ConvertHeadingWalker(child_heading));
    }
    return toc_child;
}



void NCXWriter::WriteNavPoint(const TOCModel::TOCEntry &entry, int &play_order)
{
    m_Writer->writeStartElement("navPoint");
    m_Writer->writeAttribute("id", QString("navPoint-%1").arg(play_order));
    m_Writer->writeAttribute("playOrder", QString("%1").arg(play_order));
    play_order++;
    m_Writer->writeStartElement("navLabel");
    // Compress whitespace that pretty-print may add.
    m_Writer->writeTextElement("text", entry.text.simplified());
    m_Writer->writeEndElement();
    m_Writer->writeEmptyElement("content");
    m_Writer->writeAttribute("src", entry.target);
    foreach(TOCModel::TOCEntry child, entry.children) {
        WriteNavPoint(child, play_order);
    }
    m_Writer->writeEndElement();
}


int NCXWriter::GetTOCDepth() const
{
    int max_depth = 0;
    foreach(TOCModel::TOCEntry entry, m_TOCRootEntry.children) {
        int current_depth = 0;
        TOCDepthWalker(entry , current_depth, max_depth);
    }
    return max_depth;
}


void NCXWriter::TOCDepthWalker(const TOCModel::TOCEntry &entry , int &current_depth, int &max_depth) const
{
    current_depth++;

    if (current_depth > max_depth) {
        max_depth = current_depth;
    }

    foreach(TOCModel::TOCEntry child_entry , entry.children) {
        int new_current_depth = current_depth;
        TOCDepthWalker(child_entry, new_current_depth, max_depth);
    }
}
