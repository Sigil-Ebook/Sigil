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

#ifndef TOCHTMLWRITER_H
#define TOCHTMLWRITER_H

#include "MainUI/NCXModel.h"

class QXmlStreamWriter;

/**
 * Writes the TOC into an HTML file of the EPUB publication.
 */
class TOCHTMLWriter
{
public:
    /**
     * Constructor.
     *
     * @param book The book for which we're writing the TOC.
     * @param device The IODevice into which we should write the XML.
     */
    TOCHTMLWriter(NCXModel::NCXEntry ncx_root_entry);
    ~TOCHTMLWriter();

    QString WriteXML();

private:
    /**
     *  Writes the <head> element.
     */
    void WriteHead();

    /**
     * Writes the heading entries.
     */
    void WriteHeadings();

    /**
     * Writes single heading entry; called recursively to write the TOC tree.
     *
     * @param heading The heading being written.
     * @param level   The current level of the heading
     */
    void WriteHeading(const NCXModel::NCXEntry &entry , int level);

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    QXmlStreamWriter *m_Writer;
    NCXModel::NCXEntry m_NCXRootEntry;
};

#endif // TOCHTMLWRITER_H
