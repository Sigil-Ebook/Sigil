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
#ifndef UNIVERSALUPDATES_H
#define UNIVERSALUPDATES_H

class CSSResource;
class HTMLResource;
class XMLResource;
class NCXResource;
class OPFResource;
class Resource;


class UniversalUpdates
{

public:

    // Returns a list of errors if any that occurred while loading.
    static QStringList PerformUniversalUpdates(bool resources_already_loaded,
            const QList<Resource *> &resources,
            const QHash<QString, QString> &updates,
            const QList<XMLResource *> &non_well_formed=QList<XMLResource *>());

    static std::tuple <QHash<QString, QString>,
           QHash<QString, QString>,
           QHash<QString, QString>> SeparateHtmlCssXmlUpdates(const QHash<QString, QString> &updates);

    // Made public so that ImportHTML can use it
    static void LoadAndUpdateOneCSSFile(CSSResource *css_resource,
                                        const QHash<QString, QString> &css_updates);

private:

    static QString UpdateOneHTMLFile(HTMLResource *html_resource,
                                     const QHash<QString, QString> &html_updates,
                                     const QHash<QString, QString> &css_updates);

    static void UpdateOneCSSFile(CSSResource *css_resource,
                                 const QHash<QString, QString> &css_updates);

    static QString LoadAndUpdateOneHTMLFile(HTMLResource *html_resource,
                                            const QHash<QString, QString> &html_updates,
                                            const QHash<QString, QString> &css_updates,
                                            const QList<XMLResource *> &non_well_formed=QList<XMLResource *>());

    static QString UpdateOPFFile(OPFResource *opf_resource,
                                 const QHash<QString, QString> &xml_updates);

    static QString UpdateNCXFile(NCXResource *ncx_resource,
                                 const QHash<QString, QString> &xml_updates);
};

#endif // UNIVERSALUPDATES_H
