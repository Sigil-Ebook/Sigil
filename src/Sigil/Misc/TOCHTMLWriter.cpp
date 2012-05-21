/************************************************************************
**
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

#include <QtXml/QXmlStreamWriter>

#include "TOCHTMLWriter.h"

TOCHTMLWriter::TOCHTMLWriter(NCXModel::NCXEntry ncx_root_entry)
    : m_Writer(0),
      m_NCXRootEntry(ncx_root_entry)
{
}

TOCHTMLWriter::~TOCHTMLWriter()
{
    if (m_Writer) {
        delete m_Writer;
        m_Writer = 0;
    }
}

QString TOCHTMLWriter::WriteXML()
{
    QString out;

    if (m_Writer) {
        delete m_Writer;
        m_Writer = 0;
    }
    m_Writer = new QXmlStreamWriter(&out);

    m_Writer->writeStartDocument();
    m_Writer->writeDTD( "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                        "   \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n" );
    m_Writer->writeStartElement("html");
    m_Writer->writeAttribute("xmlns", "http://www.w3.org/1999/xhtml");
    WriteHead();
    m_Writer->writeStartElement("body");
    WriteHeadings();
    m_Writer->writeEndDocument();

    return out;
}

void TOCHTMLWriter::WriteHead()
{
    m_Writer->writeStartElement("head");
    m_Writer->writeTextElement("title",  "Table of Contents");
    // Add styles as an example of how to format entries
    // Let tidy add CDATA since it must be in a special comment qxmlwriter can't do
    m_Writer->writeStartElement("style");
    m_Writer->writeAttribute("type", "text/css");

    m_Writer->writeCharacters("  p.sgc-toc-title { text-align:center; }\n");
    m_Writer->writeCharacters("  ol { list-style-type: none; }\n");
    m_Writer->writeCharacters("  li { margin-top: 0em; }\n");
    m_Writer->writeCharacters("  a { font-weight: normal; }\n");

    m_Writer->writeEndElement();
    m_Writer->writeEndElement();
}

void TOCHTMLWriter::WriteHeadings()
{
    m_Writer->writeStartElement("p");
    m_Writer->writeAttribute("class", "sgc-toc-title");
    m_Writer->writeCharacters("Table of Contents");
    m_Writer->writeEndElement();

    if (!m_NCXRootEntry.children.isEmpty())
    {
        m_Writer->writeStartElement("ol");
        m_Writer->writeAttribute("class", QString("sgc-toc-ol-%1").arg(1));

        // The TOC is written recursively;
        // WriteHeading is called for each entry in the tree
        foreach( NCXModel::NCXEntry entry, m_NCXRootEntry.children )
        {
            WriteHeading(entry, 1);
        }

        m_Writer->writeEndElement();
    }
}

void TOCHTMLWriter::WriteHeading(const NCXModel::NCXEntry &entry , int level)
{
    m_Writer->writeStartElement("li");
    m_Writer->writeAttribute("class", QString("sgc-toc-li-%1").arg(level));

    m_Writer->writeStartElement("a");
    m_Writer->writeAttribute("class", QString("sgc-toc-heading-%1").arg(level));
    m_Writer->writeAttribute("href", "../" % entry.target);
    m_Writer->writeCharacters(entry.text);
    m_Writer->writeEndElement();

    m_Writer->writeEndElement();

    if (!entry.children.isEmpty()) {
        m_Writer->writeStartElement("ol");
        m_Writer->writeAttribute("class", QString("sgc-toc-ol-%1").arg(level + 1));

        foreach (NCXModel::NCXEntry child, entry.children) {
            WriteHeading(child, level + 1);
        }

        m_Writer->writeEndElement();
    }
}
