/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#ifndef CLEANSOURCE_H
#define CLEANSOURCE_H

#include <QStringList>
#include <tidy.h>


class CleanSource
{

public:

    // Performs general cleaning (and improving)
    // of provided book XHTML source code
    static QString Clean( const QString &source );

    // No cleaning, just convert the source to valid XHTML
    static QString ToValidXHTML( const QString &source );

    static QString PrettyPrint( const QString &source );

private:

    enum TidyType
    {
        Tidy_Clean,      /**< Tidy with "clean" option (the works). */
        Tidy_Fast,       /**< Tries to run Tidy's error correcting parser
                              as fast as possible, with no unnecessary cleaning. */ 
        Tidy_PrettyPrint /**< Like Tidy_Fast, but pretty printing. */
    };

    static int RobustCSSStyleTagCount( const QString &source );

    // Cleans CSS; currently it removes the redundant CSS classes
    // that Tidy sometimes adds because it doesn't parse existing
    // CSS classes, it only adds new ones; this also merges smaller
    // style tags into larger ones
    static QString CleanCSS( const QString &source, int old_num_styles );

    // Returns the content of all CSS style tags in a list,
    // where each element is a QString representing the content
    // of a single CSS style tag
    static QStringList CSSStyleTags( const QString &source );

    // Removes empty comments that are
    // sometimes left after CDATA comments
    static QStringList RemoveEmptyComments( const QStringList &css_style_tags );

    // Merges smaller styles into bigger ones
    static QStringList MergeSmallerStyles(  const QStringList &css_style_tags );
    
    // Returns the largest index of all the Sigil CSS classes
    static int MaxSigilCSSClassIndex(       const QStringList &css_style_tags );

    static TidyDoc TidyOptions( TidyDoc tidy_document, TidyType type, int max_class_index = 0 );

    // Runs HTML Tidy on the provided XHTML source code
    static QString HTMLTidy( const QString &source, TidyType type );

    // Writes the new CSS style tags to the source, replacing the old ones
    static QString WriteNewCSSStyleTags( const QString &source, const QStringList &css_style_tags );

    // Removes redundant CSS classes from the style tags and source code;
    // Calls more specific version.
    static tuple< QString, QStringList > RemoveRedundantClasses( const QString &source, 
                                                                 const QStringList &css_style_tags );

    // Removes redundant CSS classes from the provided CSS style tags
    static QStringList RemoveRedundantClassesTags(  const QStringList &css_style_tags, 
                                                    const QHash< QString, QString > redundant_classes );

    // Removes redundant CSS classes from the provided XHTML source code;
    // Updates references to older classes that do the same thing
    static QString RemoveRedundantClassesSource(  const QString &source, 
                                                  const QHash< QString, QString > redundant_classes );

    // Returns a QHash with keys being the new redundant CSS classes,
    // and the values the old classes that already do the job of the new ones.
    static QHash< QString, QString > GetRedundantClasses( const QStringList &css_style_tags );

};


#endif // CLEANSOURCE_H


