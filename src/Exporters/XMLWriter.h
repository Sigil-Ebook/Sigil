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
#ifndef XMLWRITER_H
#define XMLWRITER_H

#include "BookManipulation/FolderKeeper.h"

class Book;
class QIODevice;
class QXmlStreamWriter;

/**
 * An abstract base class for XML writers like
 * NCXWriter and OPFWriter.
 */
class XMLWriter
{

public:

    /**
     * Constructor.
     *
     * @param book The book for which we're writing the XML.
     * @param device The IODevice into which we should write the XML.
     */
    XMLWriter(const Book &book, QIODevice &device);

    /**
     * Destructor.
     */
    virtual ~XMLWriter();

    /**
     * Writes the XML file to the disk.
     */
    virtual void WriteXML() = 0;

protected:

    /**
     * The book being exported.
     */
    const Book &m_Book;

    /**
     * The IO device that we are writing to.
     */
    QIODevice &m_IODevice;

    /**
     *  The XML writer used to write XML.
     */
    QXmlStreamWriter *m_Writer;
};

#endif // XMLWRITER_H

