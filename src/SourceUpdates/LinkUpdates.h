/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario Canada
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

#pragma once
#ifndef LINKUPDATES_H
#define LINKUPDATES_H

class HTMLResource;

class LinkUpdates
{

public:

    /**
     * Updates links in html_resources.
     * Deleting existing stylesheet links and adding new_stylesheets as links.
     *
     * @param html_resources A list of html files that need to be updated
     * @param new_stylesheets A list of the new links to add
     */

    static void UpdateLinksInAllFiles(const QList<HTMLResource *> &html_resources, const QList<QString> new_stylesheets);

private:

    static void UpdateLinksInOneFile(HTMLResource *html_resource, const QList<QString> new_stylesheets);
};

#endif // LINKUPDATES_H
