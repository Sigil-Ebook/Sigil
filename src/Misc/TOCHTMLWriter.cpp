/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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

#include "TOCHTMLWriter.h"
#include "sigil_constants.h"

const QString SGC_TOC_CSS_FILENAME = "sgc-toc.css";

TOCHTMLWriter::TOCHTMLWriter(TOCModel::TOCEntry toc_root_entry)
    :
    m_Writer(0),
    m_TOCRootEntry(toc_root_entry)
{
}

TOCHTMLWriter::~TOCHTMLWriter()
{
    if (m_Writer) {
        delete m_Writer;
        m_Writer = 0;
    }
}

QString TOCHTMLWriter::WriteXML(const QString &version)
{
    QString out;

    // Use QXmlStreamWriter to ensure correct conversion of &, <, etc.
    if (m_Writer) {
        delete m_Writer;
        m_Writer = 0;
    }

    m_Writer = new QXmlStreamWriter(&out);
    m_Writer->writeStartDocument();
    if (version.startsWith('2')) {
        m_Writer->writeDTD("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                           "   \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n");
        m_Writer->writeStartElement("html");
        m_Writer->writeAttribute("xmlns", "http://www.w3.org/1999/xhtml");
        m_Writer->writeCharacters("\n");
    } else {
        m_Writer->writeDTD("<!DOCTYPE html>\n");
        m_Writer->writeStartElement("html");
        m_Writer->writeAttribute("xmlns", "http://www.w3.org/1999/xhtml");
        m_Writer->writeAttribute("xmlns:epub", "http://www.idpf.org/2007/ops");
        m_Writer->writeCharacters("\n");
    }
    WriteHead();
    WriteBody();
    m_Writer->writeEndDocument();
    return out;
}

void TOCHTMLWriter::WriteHead()
{
    // Title
    m_Writer->writeStartElement("head");
    m_Writer->writeCharacters("\n");
    m_Writer->writeTextElement("title",  "Contents");
    m_Writer->writeCharacters("\n");

    // TOC
    m_Writer->writeStartElement("link");
    m_Writer->writeAttribute("href", "../Styles/" % SGC_TOC_CSS_FILENAME);
    m_Writer->writeAttribute("rel", "stylesheet");
    m_Writer->writeAttribute("type", "text/css");
    m_Writer->writeEndElement();
    m_Writer->writeCharacters("\n");

    // End head
    m_Writer->writeEndElement();
    m_Writer->writeCharacters("\n");
}

void TOCHTMLWriter::WriteBody()
{
    m_Writer->writeStartElement("body");
    m_Writer->writeCharacters("\n");
    // Page heading
    m_Writer->writeStartElement("div");
    m_Writer->writeAttribute("class", "sgc-toc-title");
    m_Writer->writeCharacters(QObject::tr("Table of Contents"));
    m_Writer->writeEndElement();
    m_Writer->writeCharacters("\n");
    // Entries
    WriteEntries(m_TOCRootEntry);
}

void TOCHTMLWriter::WriteEntries(TOCModel::TOCEntry parent_entry, int level)
{
    foreach(TOCModel::TOCEntry entry, parent_entry.children) {
        m_Writer->writeStartElement("div");
        m_Writer->writeAttribute("class", "sgc-toc-level-" % QString::number(level));
        m_Writer->writeCharacters("\n");
        m_Writer->writeCharacters("  ");
        m_Writer->writeStartElement("a");
        m_Writer->writeAttribute("href", "../" + entry.target);
        m_Writer->writeCharacters(entry.text);
        m_Writer->writeEndElement();
        m_Writer->writeCharacters("\n");
        // Recursively write out subheadings
        WriteEntries(entry, level + 1);
        m_Writer->writeEndElement();
        m_Writer->writeCharacters("\n");
    }
}
