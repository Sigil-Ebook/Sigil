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

#include "Misc/CSSInfo.h"

CSSInfo::CSSInfo( const QString &text, bool isCSSFile )
{
    if (isCSSFile) {
        parseCSSSelectors(text, 0, 0);
    }
    else {
        // Is an HTML file with possibly some inline CSS within it
        QRegExp inline_styles_search("<\\s*style\\s[^>]+>", Qt::CaseInsensitive);
        inline_styles_search.setMinimal(true);

        int offset = inline_styles_search.indexIn(text);
        if ( offset > 0 ) {
            offset += inline_styles_search.matchedLength();
            int end = text.indexOf(QRegExp("<\\s*/\\s*style\\s*>", Qt::CaseInsensitive), offset);
            if (end >= offset) {
                int line = text.left(offset).count(QChar('\n'));
                parseCSSSelectors(text.mid(offset, end - offset), line, offset);
            }
        }
    }
}

QList< CSSInfo::CSSSelector* > CSSInfo::getClassSelectors(const QString filterClassName)
{
    QList<CSSInfo::CSSSelector*> selectors;
    foreach(CSSInfo::CSSSelector *cssSelector, m_CSSSelectors) {
        if (cssSelector->classNames.count() > 0) {
            if (filterClassName.isEmpty() || cssSelector->classNames.contains(filterClassName)) {
                selectors.append(cssSelector);
            }
        }
    }
    return selectors;
}

CSSInfo::CSSSelector* CSSInfo::getCSSSelectorForElementClass( const QString &elementName, const QString &className )
{
    if (!className.isEmpty()) {
        // Find the selector(s) if any with this class name
        QList< CSSInfo::CSSSelector* > class_selectors = getClassSelectors(className);
        if (class_selectors.count() > 0) {
            // First look for match on element and class
            foreach(CSSInfo::CSSSelector *cssSelector, class_selectors) {
                if (cssSelector->elementNames.contains(elementName)) {
                    return cssSelector;
                }
            }
            // Just return the first match on class.
            return class_selectors.at(0);
        }
    }
    // try match on element name alone
    foreach(CSSInfo::CSSSelector *cssSelector, m_CSSSelectors) {
        if (cssSelector->elementNames.contains(elementName)) {
            return cssSelector;
        }
    }
    return NULL;
}

void CSSInfo::parseCSSSelectors( const QString &text, const int &offsetLines, const int &offsetPos )
{
    QRegExp strip_attributes_regex("\\[[^\\]]*\\]");
    QRegExp strip_ids_regex("#[^\\s\\.]+");
    QRegExp strip_non_name_chars_regex("[^A-Za-z0-9_\\-\\.]+");

    QString search_text = ReplaceBlockComments(text);

    // CSS selectors can be in a myriad of formats... the class based selectors could be:
    //    .c1 / e1.c1 / e1.c1.c2 / e1[class~=c1] / e1#id1.c1 / e1.c1#id1 / .c1, .c2 / ...
    // Then the element based selectors could be:
    //    e1 / e1 > e2 / e1 e2 / e1 + e2 / e1[attribs...] / e1#id1 / e1, e2 / ...
    // Really needs a parser to do this properly, this will only handle the 90% scenarios.

    int pos = 0;
    int open_brace_pos = -1;
    int close_brace_pos = -1;
    while (true) {
        open_brace_pos = search_text.indexOf(QChar('{'), pos);
        if (open_brace_pos < 0) {
            break;
        }
        // Now search backwards until we get a line containing text.
        bool have_text = false;
        pos = open_brace_pos - 1;
        while ((pos >= 0) && (search_text.at(pos) != QChar('\n') || !have_text)) {
            if (search_text.at(pos).isLetter()) {
                have_text = true;
            }
            pos--;
        }
        pos++;
        if (!have_text) {
            // Really badly formed CSS document - try to skip ahead
            pos = open_brace_pos + 1;
            continue;
        }
        close_brace_pos = search_text.indexOf(QChar('}'), open_brace_pos + 1);
        if (close_brace_pos < 0) {
            // Another badly formed scenario - no point in looking further
            break;
        }

        // Skip past leading whitespace/newline.
        while (search_text.at(pos).isSpace()) {
            pos++;
        }
        int line = search_text.left(pos + 1).count(QChar('\n')) + 1;

        QString selector_text = search_text.mid(pos, open_brace_pos - pos).trimmed();

        // Handle case of a selector group containing multiple declarations
        QStringList matches = selector_text.split(QChar(','), QString::SkipEmptyParts);
        foreach (QString match, matches) {
            CSSSelector *selector = new CSSSelector();

            selector->position = pos + offsetPos;
            selector->line = line + offsetLines;
            selector->isGroup = matches.length() > 1;
            selector->openingBracePos = open_brace_pos + offsetPos;
            selector->closingBracePos = close_brace_pos + offsetPos;

            // Need to parse our selector text to determine what sort of selector it contains.
            // First strip out any attributes and then identifiers
            match.replace(QRegExp(strip_attributes_regex), "");
            match.replace(QRegExp(strip_ids_regex), "");
            // Also replace any other characters like > or + not of interest
            match.replace(QRegExp(strip_non_name_chars_regex), " ");

            // Now break it down into the element components
            QStringList elements = match.trimmed().split(QChar(' '), QString::SkipEmptyParts);
            foreach (QString element, elements) {
                if (element.contains(QChar('.'))) {
                    QStringList parts = element.split('.');
                    if (!parts.at(0).isEmpty()) {
                        selector->elementNames.append(parts.at(0));
                    }
                    for (int i=1; i <parts.length(); i++) {
                        selector->classNames.append(parts.at(i));
                    }
                }
                else {
                    selector->elementNames.append(element);
                }
            }

            m_CSSSelectors.append(selector);
        }
        pos = open_brace_pos + 1;
    }
}

QString CSSInfo::ReplaceBlockComments(const QString &text)
{
    // We take a copy of the text and remove all block comments from it.
    // However we must be careful to replace with spaces/keep line feeds
    // so that do not corrupt the position information used by the parser.
    QString new_text(text);
    QRegExp comment_search("/\\*.*\\*/");
    comment_search.setMinimal(true);
    int start = 0;
    int comment_index;
    while (true) {
        comment_index = comment_search.indexIn(new_text, start);
        if (comment_index < 0) {
            break;
        }
        QString match_text = new_text.mid(comment_index, comment_search.matchedLength());
        match_text.replace(QRegExp("[^\r\n]"), " ");
        new_text.remove(comment_index, match_text.length());
        new_text.insert(comment_index, match_text);

        // Prepare for the next comment.
        start = comment_index + comment_search.matchedLength();
        if (start >= new_text.length() - 2) {
            break;
        }
    }
    return new_text;
}
