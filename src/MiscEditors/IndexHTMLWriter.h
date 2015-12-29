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

#ifndef INDEXWRITER_H
#define INDEXWRITER_H

#include <QStandardItem>

/**
 * Writes the Index into an HTML file of the EPUB publication.
 */
class IndexHTMLWriter
{
public:
    IndexHTMLWriter();

    QString WriteXML(const QString &version);

private:
    void WriteEntries(QStandardItem *parent_item = NULL);

    QString m_IndexHTMLFile;
};

#endif // INDEXWRITER_H
