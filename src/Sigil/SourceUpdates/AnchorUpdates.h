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
#ifndef ANCHORUPDATES_H
#define ANCHORUPDATES_H

class HTMLResource;
class NCXResource;

class AnchorUpdates
{

public:

    static void UpdateAllAnchorsWithIDs( const QList< HTMLResource* > &html_resources );

    /**
     * Updates the anchors in html_resources that point to ids that were originally located in originating_filename
     * but are now distributed over the files referenced by new_files.
     *
     * @param html_resources A list of xhtml files that need to be scanned for references to originating_filename.
     * @param originating_filename The name of the original file for which references need to be reconciled.
     * @param new_files A list of the new files created by splitting the originating_filename.
     */
    static void UpdateExternalAnchors( const QList< HTMLResource* > &html_resources, const QString &originating_filename, const QList< HTMLResource* > new_files );

    /**
     * Updates the src attributes of the content tags in the toc.ncx file that point to 
     * ids that were originally located in originating_filename
     * but are now distributed over the files referenced by new_files.
     *
     * @param ncx_resource The TOC file
     * @param originating_filename The name of the original file for which references need to be reconciled.
     * @param new_files A list of the new files created by splitting the originating_filename.
     */
    static void UpdateTOCEntries(NCXResource* ncx_resource, const QString &originating_filename, const QList< HTMLResource* > new_files);

private:

    static QHash< QString, QString > GetIDLocations( const QList< HTMLResource* > &html_resources );

    static tuple< QString, QList< QString > > GetOneFileIDs( HTMLResource* html_resource );

    static void UpdateAnchorsInOneFile( HTMLResource *html_resource, 
                                        const QHash< QString, QString > ID_locations );

    static void UpdateExternalAnchorsInOneFile( HTMLResource *html_resource, const QString &originating_filename, const QHash< QString, QString > id_locations );
};

#endif // ANCHORUPDATES_H