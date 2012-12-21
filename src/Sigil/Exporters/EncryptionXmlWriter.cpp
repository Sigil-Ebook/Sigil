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

#include <QtCore/QXmlStreamWriter>

#include "BookManipulation/Book.h"
#include "Exporters/EncryptionXmlWriter.h"
#include "Misc/Utility.h"

EncryptionXmlWriter::EncryptionXmlWriter(const Book &book, QIODevice &device)
    : XMLWriter(book, device)
{
}

void EncryptionXmlWriter::WriteXML()
{
    m_Writer->writeStartDocument();
    m_Writer->writeStartElement("encryption");
    m_Writer->writeAttribute("xmlns", "urn:oasis:names:tc:opendocument:xmlns:container");
    m_Writer->writeAttribute("xmlns:enc", "http://www.w3.org/2001/04/xmlenc#");
    QList< FontResource * > font_resources = m_Book.GetFolderKeeper().GetResourceTypeList< FontResource >();
    foreach(FontResource * font_resource, font_resources) {
        WriteEncryptedData(*font_resource);
    }
    m_Writer->writeEndElement();
    m_Writer->writeEndDocument();
}


void EncryptionXmlWriter::WriteEncryptedData(const FontResource &font_resource)
{
    QString algorithm = font_resource.GetObfuscationAlgorithm();

    if (algorithm.isEmpty()) {
        return;
    }

    m_Writer->writeStartElement("enc:EncryptedData");
    m_Writer->writeEmptyElement("enc:EncryptionMethod");
    m_Writer->writeAttribute("Algorithm", algorithm);
    m_Writer->writeStartElement("enc:CipherData");
    m_Writer->writeEmptyElement("enc:CipherReference");
    m_Writer->writeAttribute("URI", font_resource.GetRelativePathToRoot());
    m_Writer->writeEndElement();
    m_Writer->writeEndElement();
}
