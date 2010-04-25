/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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

#include "../BookManipulation/Headings.h"
#include <QSharedPointer>

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
    static void GiveIDsToHeadings( QList< HTMLResource* > html_resources );

    /**
     * Gives ID's to all headings that don't have them
     * in the specified resource.
     *
     * @param html_resource The HTMLResource on whose headings 
     *                      the operation will be performed.
     */
    static void GiveIDsToHeadingsInResource( HTMLResource *html_resource );

    /**
     * Returns the maximum Sigil heading ID index. 
     * 
     * @param headings The list of headings to search.
     * @return The highest index.
     */
    static int MaxSigilHeadingIDIndex( const QList< Headings::Heading > headings );

    /**
     * Returns the cover page from the HTML resources.
     *
     * @param html_resource The book's HTML resources.
     * @return The cover page resource.
     */
    static HTMLResource* GetCoverPage( QList< HTMLResource* > html_resources );

    /**
     * Determines if a cover page exists.
     *
     * @param html_resource The book's HTML resources.
     * @return \c true if a cover page exists.
     */
    static bool CoverPageExists( QList< HTMLResource* > html_resources );

    /**
     * Uses heuristics to try and guess which of the
     * HTML resources is a cover page. If it finds 
     * a resource that matches, it sets it as the cover page.
     *
     * @param html_resource The book's HTML resources.
     */
    static void TryToSetCoverPage( QList< HTMLResource* > html_resources );
    
    /**
     * Determines if a cover image exists.
     *
     * @param image_resources The book's image resources.
     * @return \c true if a cover image exists.
     */
    static bool CoverImageExists( QList< ImageResource* > image_resources );

    /**
     * Uses heuristics to try and guess which of the 
     * image resources is a cover image. If it finds 
     * a resource that matches, it sets it as the cover page.
     *
     * @param html_resource The book's HTML resources.
     * @param image_resources The book's image resources.
     */
    static void TryToSetCoverImage( QList< HTMLResource* > html_resources,
                                    QList< ImageResource* > image_resources );

    /**
     * Determines if a flow is under the specified threshold.
     * Used as a heuristic for finding the cover XHTML file.
     *
     * @param resource The XHTML flow to inspect.
     * @param threshold The maximum number of text characters.
     * @return \c true if the text of the HTML resource specified 
     *         has fewer characters than 'threshold' number. 
     */
    static bool IsFlowUnderThreshold( HTMLResource *html_resource, int threshold );
};

#endif // BOOKNORMALIZATION_H

