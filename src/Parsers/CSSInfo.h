/************************************************************************
**
**  Copyright (C) 2016-2022 Kevin B. Hendricks, Stratford, ON, Canada
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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
#ifndef CSSINFO_H
#define CSSINFO_H

#include <QObject>
#include <QStringList>
#include "Parsers/qCSSParser.h"

class CSSInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * Parse the supplied css text
     */
    CSSInfo(const QString &text, int offset = 0);

    ~CSSInfo();

    struct CSSSelector {
        int pos;                    /* The position in the file of the full selector name          */
        QString text;               /* The text of this selector                  */
        QString className;          /* The classname(s) (stripped of periods) if a class selector  */
        QString elementName;        /* The element names if any (stripped of any ids/attributes)   */

        bool operator<(const CSSSelector &rhs) const {
            return pos < rhs.pos;
        }
    };

    QList<CSSSelector*> getAllSelectors();

    /**
     * Return selectors subset for only class based CSS declarations.
     */
    QList<CSSSelector *> getClassSelectors(const QString filterClassName = "");

    /**
     * Search for a matching class selector  given an element name and an optional
     * class name for the style.
     * Looks in order of: elementName.style, .style
     */
    CSSSelector *getCSSSelectorForElementClass(const QString &elementName, const QString &className);

    /**
     * Search for *all* CSS selector that match an elementName, and classname
     * not just the first.  Needed because one use of an html class can actually 
     * relate to more than one style
     */

    QList<CSSSelector *> getAllCSSSelectorsForElementClass(const QString &elementName, const QString &className);

    /**
     * Return a list of all property values for the given property in the CSS.
     */
    QStringList getAllPropertyValues(QString property);

    /**
     * Return the original text with a reformatted appearance either to
     * a multiple line style (each property on its own line) or single line style.
     */
    QString getReformattedCSSText(bool multipleLineFormat);

    /**
     * Search for a CSSSelector with the same definition of original group text and pos as this,
     * and if found remove from the document text
     * If not found returns a null string.
     * Note the caller must intialise a new CSSInfo object to re-parse the updated text for another remove.
     */
    QString removeMatchingSelectors(QList<CSSSelector *> cssSelectors);

    // QString replaceBlockComments(const QString &text);

private:
    void parseStyles(const QString &text, int offsetPos);
    void generateSelectorsList();

    QList<CSSSelector *> m_CSSSelectors;
    QVector<CSSParser::token> m_csstokens;

    QString m_source;
    int m_posoffset;
};

template<class T>
bool dereferencedLessThan(T *o1, T *o2)
{
    return *o1 < *o2;
}

#endif // CSSINFO_H

