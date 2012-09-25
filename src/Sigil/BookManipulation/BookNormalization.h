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
#ifndef BOOKNORMALIZATION_H
#define BOOKNORMALIZATION_H

#include <QtCore/QSharedPointer>

#include "BookManipulation/Headings.h"

class Book;
class HTMLResource;
class ImageResource;

/**
 * Houses the book normalization process.
 */
class BookNormalization
{

public:

    /**
     * Normalizes the book's contents for export.
     * Performs all the operations necessary 
     * on the Book before it can be exported,
     * like adding ID's to all headings etc.
     *
     * @param book The book to normalize.
     */
    static void Normalize( QSharedPointer< Book > book ); 

private:

    /**
     * Gives ID's to all headings that don't have them.
     *
     * @param html_resources All the book's html resources.
     */
    static void GiveIDsToHeadings( const QList< HTMLResource* > &html_resources, int toc_id_index );

    /**
     * Gives ID's to all headings that don't have them
     * in the specified resource.
     *
     * @param html_resource The HTMLResource on whose headings 
     *                      the operation will be performed.
     */
    static int GiveIDsToHeadingsInResource( HTMLResource *html_resource, int toc_id_index );

    static int MaxSigilHeadingIDIndex(QHash<QString, QStringList> file_ids);

    static void RemoveTOCIDs( const QList< HTMLResource* > &html_resources, QStringList &used_ids );
    static void RemoveTOCIDsInResource( HTMLResource* html_resources, QStringList &used_ids );
    static void RemoveTOCIDsInNodes( xc::DOMNode &node, QStringList &used_ids );
};

#endif // BOOKNORMALIZATION_H

