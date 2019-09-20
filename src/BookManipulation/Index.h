/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks Stratford, ON, Canada 
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
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
#ifndef INDEX_H
#define INDEX_H

class HTMLResource;

/**
 * Houses the Index process.
 * Ids are added via static routines to all files if text matches Index settings.
 * Patterns are read from the Index dialog model and written to the Index Entry storage.
 */
class Index
{

public:
    static bool BuildIndex(QList<HTMLResource *> html_resources);

private:
    static void AddIndexIDsOneFile(HTMLResource *html_resource);

    static bool CreateIndexEntry(const QString text, HTMLResource *html_resource, QString index_id_name, bool is_custom_index_entry, QString custom_index_name);
};

#endif // INDEX_H
