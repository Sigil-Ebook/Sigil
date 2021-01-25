/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef HTMLSTYLEINFO_H
#define HTMLSTYLEINFO_H

#include <QObject>
#include <QStringList>
#include "Parsers/CSSInfo.h"

class QStringList;

class HTMLStyleInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * Parse the supplied css text
     */
    HTMLStyleInfo(const QString &htmltext);

    ~HTMLStyleInfo();

    struct CSSProperty {
        QString name;
        QString value;
    };


    bool hasStyles() { return m_styles.size() > 0; };

    QList<CSSInfo::CSSSelector *> getAllSelectors();

    /**
     * Search for a matching class selector  given an element name and an optional
     * class name for the style.
     * Looks in order of: elementName.style, .style
     */
    CSSInfo::CSSSelector *getCSSSelectorForElementClass(const QString &elementName, const QString &className);

    /**
     * Search for *all* CSS selector that match an elementName, and classname
     * not just the first.  Needed because one use of an html class can actually 
     * relate to more than one style
     */

    QList<CSSInfo::CSSSelector *> getAllCSSSelectorsForElementClass(const QString &elementName, const QString &className);

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
     * Note the caller must intialise a new HTMLStyleInfo object to re-parse the updated text for another remove.
     */
    QString removeMatchingSelectors(QList<CSSInfo::CSSSelector *> cssSelectors);

    static QList<CSSProperty> getCSSProperties(const QString &text, const int &styleTextStartPos, const int &styleTextEndPos);
    static QString formatCSSProperties(QList<CSSProperty> new_properties, bool multipleLineFormat, const int &selectorIndent = 0);

private:
    bool findInlineStyleBlock(const QString &text, int offset, int &styleStart, int &styleEnd);
    void generateSelectorsList();

    QList<CSSInfo *> m_styles;
    QList<int> m_starts;
    QList<int> m_lengths;
    QString m_source;
};

#endif // HTMLSTYLEINFO_H
