/************************************************************************
**
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

#include <QtCore/QObject>
#include <QtCore/QStringList>

class QStringList;

class CSSInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * Parse the supplied text to identify all named CSS styles.
     * The supplied text can be a CSS file or html containing inline styles.
     */
    CSSInfo(const QString &text, bool isCSSFile = true);

    struct CSSSelector {
        QString originalText;       /* The original text of the complete selector                  */
        bool isGroup;               /* Whether selector was declared in a comma separated group    */
        QString groupText;          /* The text of the selector for this group (same as originalText if not a group) */
        QStringList classNames;     /* The classname(s) (stripped of periods) if a class selector  */
        QStringList elementNames;   /* The element names if any (stripped of any ids/attributes)   */
        int line;                   /* For convenience of navigation, the line# selector is on     */
        int position;               /* The position in the file of the full selector name          */
        int openingBracePos;        /* Location of the opening brace which contains properties     */
        int closingBracePos;        /* Location of the closing brace which contains properties     */

        bool operator<(const CSSSelector &rhs) const {
            return line < rhs.line;
        }
    };

    struct CSSProperty {
        QString name;
        QString value;
    };

    /**
     * Return selectors subset for only class based CSS declarations.
     */
    QList< CSSSelector * > getClassSelectors(const QString filterClassName = "");

    /**
     * Search for a line position for a tag element name and an optional
     * class name for the style.
     * Looks in order of: elementName.style, .style
     */
    CSSSelector *getCSSSelectorForElementClass(const QString &elementName, const QString &className);

     
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
     * Search for a CSSSelector with the same definition of original group text and line as this,
     * and if found remove from the document text, returning the new text.
     * If not found returns a null string.
     * Note the caller must intialise a new CSSInfo object to re-parse the updated text for another remove.
     */
    QString removeMatchingSelectors(QList<CSSSelector *> cssSelectors);

    static QList< CSSProperty * > getCSSProperties(const QString &text, const int &styleTextStartPos, const int &styleTextEndPos);
    static QString formatCSSProperties(QList< CSSProperty * > new_properties, bool multipleLineFormat, const int &selectorIndent = 0);

private:
    bool findInlineStyleBlock(const QString &text, const int &offset, int &styleStart, int &styleEnd);
    void parseCSSSelectors(const QString &text, const int &offsetLines, const int &offsetPos);
    QString replaceBlockComments(const QString &text);

    QList< CSSSelector * > m_CSSSelectors;
    QString m_OriginalText;
    bool m_IsCSSFile;
};

template<class T>
bool dereferencedLessThan(T *o1, T *o2)
{
    return *o1 < *o2;
}

#endif // CSSINFO_H

