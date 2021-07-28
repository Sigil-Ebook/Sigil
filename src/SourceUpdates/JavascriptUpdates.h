/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, Ontario Canada
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
#ifndef JAVASCRIPTUPDATES_H
#define JAVASCRIPTUPDATES_H

class HTMLResource;

class JavascriptUpdates
{

public:

    /**
     * Updates javascripts linked into html_resources.
     * Deleting existing javascript linked in and adding new_javascripts.
     *
     * @param html_resources A list of html files that need to be updated
     * @param new_javascripts A list of the new links to add
     */

    static void UpdateJavascriptsInAllFiles(const QList<HTMLResource *> &html_resources, const QList<QString> new_javascripts);

private:

    static void UpdateJavascriptsInOneFile(HTMLResource *html_resource, const QList<QString> new_javascripts);
};

#endif // JAVASCRIPTUPDATES_H
