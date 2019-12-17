/************************************************************************
**
**  Copyright (C) 2016-2019 Kevin B. Hendricks, Stratford, ON Canada
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
**  Copyright (C) 2012      Grant Drake
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

#include "Misc/EmbeddedPython.h"

#include <QRegularExpression>
#include "Misc/Utility.h"
#include "Misc/CSSInfo.h"

const int TAB_SPACES_WIDTH = 4;
const QString LINE_MARKER("[SIGIL_NEWLINE]");
static const QString DELIMITERS = "}{;";


// Note: CSSProperties and CSSSelectors are simple struct that this code
// created with new and so need to be manually cleaned up to prevent
// large memory leaks

CSSInfo::CSSInfo(const QString &text, bool isCSSFile)
    : m_OriginalText(text),
      m_IsCSSFile(isCSSFile)
{
    if (isCSSFile) {
        parseCSSSelectors(text, 0, 0);
    } else {
        // This is an HTML file with any number of inline CSS style blocks within it
        int style_start = -1;
        int style_end = -1;
        int offset = 0;

        while (findInlineStyleBlock(text, offset, style_start, style_end)) {
            int line = text.left(style_start).count(QChar('\n'));
            parseCSSSelectors(text.mid(style_start, style_end - style_start), line, style_start);
            offset = style_end;
        }
    }
}

// Need to manually clean up the Selector List
CSSInfo::~CSSInfo()
{
  foreach(CSSSelector * sp, m_CSSSelectors) {
      if (sp) delete sp;
  } 
}


QList<CSSInfo::CSSSelector *> CSSInfo::getClassSelectors(const QString filterClassName)
{
    QList<CSSInfo::CSSSelector *> selectors;
    foreach(CSSInfo::CSSSelector * cssSelector, m_CSSSelectors) {
        if (cssSelector->classNames.count() > 0) {
            if (filterClassName.isEmpty() || cssSelector->classNames.contains(filterClassName)) {
                selectors.append(cssSelector);
            }
        }
    }
    return selectors;
}

CSSInfo::CSSSelector *CSSInfo::getCSSSelectorForElementClass(const QString &elementName, const QString &className)
{
    if (!className.isEmpty()) {
        // Find the selector(s) if any with this class name
        QList<CSSInfo::CSSSelector *> class_selectors = getClassSelectors(className);

        if (class_selectors.count() > 0) {
            // First look for match on element and class
            foreach(CSSInfo::CSSSelector * cssSelector, class_selectors) {
                // Always match on wildcard class selector
                if (cssSelector->elementNames.isEmpty()) {
                    return cssSelector;
                }
                if (cssSelector->elementNames.contains(elementName)) {
                    // Doublecheck that the full element.class is actually in the text
                    // to avoid, e.g.,  div class="test" matching p.test + div
                    if (cssSelector->groupText.contains(elementName % "." % className)) {
                        return cssSelector;
                    }
                }
            }
        }
    } else {

        // try match on element name alone
        foreach(CSSInfo::CSSSelector * cssSelector, m_CSSSelectors) {
            if (cssSelector->elementNames.contains(elementName) && cssSelector->classNames.isEmpty()) {
                return cssSelector;
            }
        }
    }
    return NULL;
}

QList<CSSInfo::CSSSelector *> CSSInfo::getAllCSSSelectorsForElementClass(const QString &elementName, const QString &className)
{
    QList<CSSInfo::CSSSelector *> matches;
    if (!className.isEmpty()) {
        // Find the selector(s) if any with this class name
        QList<CSSInfo::CSSSelector *> class_selectors = getClassSelectors(className);

        if (class_selectors.count() > 0) {
            // First look for match on element and class
            foreach(CSSInfo::CSSSelector * cssSelector, class_selectors) {
                // Always match on wildcard class selector
                if (cssSelector->elementNames.isEmpty()) {
                    matches.append(cssSelector);;
                }
                if (cssSelector->elementNames.contains(elementName)) {
                    // Doublecheck that the full element.class is actually in the text
                    // to avoid, e.g.,  div class="test" matching p.test + div
                    if (cssSelector->groupText.contains(elementName % "." % className)) {
                        matches.append(cssSelector);
                    }
                }
            }
        }
    } else {
        // try match on element name alone
        foreach(CSSInfo::CSSSelector * cssSelector, m_CSSSelectors) {
            if (cssSelector->elementNames.contains(elementName) && cssSelector->classNames.isEmpty()) {
                matches.append(cssSelector);
            }
        }
    }
    return matches;
}


QStringList CSSInfo::getAllPropertyValues(QString property)
{
    QStringList property_values;

    int last_selector_line = -1;

    for (int i = m_CSSSelectors.count() - 1; i >= 0; i--) {
        CSSInfo::CSSSelector *cssSelector = m_CSSSelectors.at(i);

        if (cssSelector->isGroup && cssSelector->line == last_selector_line) {
            // Must be a selector group which we have already processed.
            continue;
        }

        last_selector_line = cssSelector->line;

        QList<CSSInfo::CSSProperty *> properties = getCSSProperties(m_OriginalText, cssSelector->openingBracePos + 1, cssSelector->closingBracePos);
        foreach (CSSInfo::CSSProperty *p, properties) {
            // If property is empty return properties of everything
            if (property.isEmpty() || p->name == property) {
                property_values.append(p->value);
            }
	    delete p;
        }
    }

    return property_values;
}

QString CSSInfo::getReformattedCSSText(bool multipleLineFormat)
{
    QString csstext(m_OriginalText);

    // Note, the EmbeddedPython interface does not handle bool properly
    // So convert to int with 0 or 1 value for the time being
    int useoneline = 1;
    if (multipleLineFormat) useoneline = 0;

    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(csstext));
    args.append(QVariant(useoneline));
    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("cssreformatter"),
                                         QString("reformat_css"),
                                         args,
                                         &rv,
                                         error_traceback);
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("Error in cssreformatter: ") + QString::number(rv), 
				         error_traceback);
        // an error happened, return unchanged original
        return QString(csstext);
    }
    // QVariant results are a String List (new_css_text, errors, warnings)
    QStringList results = res.toStringList();
    QString new_csstext = results[0];
    QString errors = results[1];
    QString warnings = results[2];

    if (!errors.isEmpty()) {
        Utility::DisplayStdWarningDialog(QString("Error in cssreformatter: "), errors);
        // an error happened, return unchanged original
        return QString(csstext);
    }

    if (!warnings.isEmpty()) {
        Utility::DisplayStdWarningDialog(QString("Warnings from cssreformatter: "), warnings);
    }

    return new_csstext;

#if 0  // attempt to replace out broken css reformatter with a python one based on css_parser
    int selector_indent = m_IsCSSFile ? 0 : TAB_SPACES_WIDTH;
    // Work backwards through our selectors to change the document as will be in line order.
    // must also cater for fact that a selector group ( e.g. body,p { ) will have multiple
    // selector entries so skip if the next selector is on the same line.
    int last_selector_line = -1;

    for (int i = m_CSSSelectors.count() - 1; i >= 0; i--) {
        CSSInfo::CSSSelector *cssSelector = m_CSSSelectors.at(i);

        if (cssSelector->isGroup && cssSelector->line == last_selector_line) {
            // Must be a selector group which we have already processed.
            continue;
        }

        last_selector_line = cssSelector->line;

        // Will place a blank line after every style if in multi-line mode
        if (multipleLineFormat) {
            new_text.insert(cssSelector->closingBracePos + 1, LINE_MARKER);
        }

        // Now replace the contents inside the braces
        QList<CSSInfo::CSSProperty *> new_properties = getCSSProperties(m_OriginalText, cssSelector->openingBracePos + 1, cssSelector->closingBracePos);
        const QString &new_properties_text = formatCSSProperties(new_properties, multipleLineFormat, selector_indent);
        new_text.replace(cssSelector->openingBracePos + 1, cssSelector->closingBracePos - cssSelector->openingBracePos - 1, new_properties_text);
	// clear up new_properties as they were created with new
	foreach(CSSInfo::CSSProperty* p, new_properties) {
	    if (p) delete p;
	}
        // Reformat the selector text itself - whitespace only since incomplete parsing.
        // Will ensure the braces are placed on the same line as the selector name,
        // comma separated groups are spaced apart and double spaces are removed.
        QString selector_text = m_OriginalText.mid(cssSelector->position, cssSelector->openingBracePos - cssSelector->position);
        selector_text.replace(QRegularExpression(","), ", ");
        selector_text.replace(QRegularExpression(" {2,}"), QChar(' '));
        new_text.replace(cssSelector->position, cssSelector->openingBracePos - cssSelector->position, selector_text.trimmed() % QChar(' '));

        // Make sure the selector itself is left-aligned (indented if inline CSS)
        if (cssSelector->position > 0) {
            int pos = cssSelector->position;

            while (pos-- > 0 && (new_text.at(pos) == QChar(' ') || new_text.at(pos) == QChar('\t')));

            if (pos <= 0) {
                new_text.replace(0, cssSelector->position, QString(" ").repeated(selector_indent));
            } else {
                new_text.replace(pos + 1, cssSelector->position - pos - 1, QString(" ").repeated(selector_indent));
                new_text.insert(pos + 1, QChar('\n'));
            }
        }
    }

    // Finally remove extra blank lines so styles are placed consecutively if inline, or one line spacing if CSS.
    // If we are reformatting an inline CSS, make sure we are doing so only within style blocks
    if (m_IsCSSFile) {
        new_text.replace(QRegularExpression("\n{2,}"), "\n");
    } else {
        int style_start = -1;
        int style_end = -1;
        int offset = 0;

        while (findInlineStyleBlock(new_text, offset, style_start, style_end)) {
            QString script_text = new_text.mid(style_start, style_end - style_start);
            script_text.replace(QRegularExpression("\n{2,}"), "\n");
            new_text.replace(style_start, style_end - style_start, script_text);
            offset = style_start + script_text.length();
        }
    }

    new_text.replace(LINE_MARKER, "\n");
    return new_text.trimmed();
#endif
}

QString CSSInfo::removeMatchingSelectors(QList<CSSSelector *> cssSelectors)
{
    // First try to find a CSS selector currently parsed that matches each of the selectors supplied.
    QList<CSSSelector *> remove_selectors;
    foreach(CSSSelector * css_selector, cssSelectors) {
        foreach(CSSSelector * match_selector, m_CSSSelectors) {
            if ((match_selector->line == css_selector->line) &&
                (match_selector->groupText == css_selector->groupText)) {
                remove_selectors.append(match_selector);
                break;
            }
        }
    }

    // If no matches found, return a null string to caller
    if (remove_selectors.isEmpty()) {
        return QString();
    }

    QString new_text(m_OriginalText);
    // Sort the selectors by line number ascending.
    qSort(remove_selectors.begin(), remove_selectors.end(), dereferencedLessThan<CSSSelector>);
    CSSSelector *remove_selector;

    // Now iterate in reverse order
    for (int i = remove_selectors.count() - 1; i >= 0; i--) {
        remove_selector = remove_selectors.at(i);

        // Is the selector in a group - if so, just remove the text portion.
        if (remove_selector->isGroup) {
            // Life is now complicated. We need to be careful how we remove the text.
            const int selector_length = remove_selector->originalText.length();
            const QString current_selector_text = new_text.mid(remove_selector->position, selector_length);
            QStringList current_groups = current_selector_text.split(QChar(','), QString::SkipEmptyParts);

            // If we are the last group within the selector, we can safely remove the whole thing
            if (current_groups.count() > 1) {
                // Darn, we aren't. We will reassemble the group selector, calculate the difference in length
                // between the old and new and then update all of the selectors for the group to assign a
                // new offset for the opening bracket/closing bracket position.
                for (int j = 0; j < current_groups.count(); j++) {
                    if (current_groups.at(j).trimmed() == remove_selector->groupText) {
                        current_groups.removeAt(j);
                        break;
                    }
                }

                const QString new_groups_text = current_groups.join(",").trimmed();
                int delta = remove_selector->originalText.length() - new_groups_text.length();
                foreach(CSSSelector * update_selector, m_CSSSelectors) {
                    if (update_selector->line == remove_selector->line) {
                        update_selector->openingBracePos -= delta;
                        update_selector->closingBracePos -= delta;
                    }
                }
                new_text.replace(remove_selector->position, selector_length, new_groups_text);
                // Done all we intend to for this selector group for now
                continue;
            }
        }

        // Remove the entire text for this CSS style from the stylesheet, plus trailing whitespace.
        int start_pos = remove_selector->position;
        int end_pos = remove_selector->closingBracePos;

        while (++end_pos < new_text.length() && new_text.at(end_pos).isSpace());

        new_text.remove(start_pos, end_pos - start_pos);
    }

    return new_text;
}

QList<CSSInfo::CSSProperty *> CSSInfo::getCSSProperties(const QString &text, const int &styleTextStartPos, const int &styleTextEndPos)
{
    QList<CSSProperty *> new_properties;

    if (styleTextEndPos - 1 <= styleTextStartPos) {
        return new_properties;
    }

    const QString &style_text = text.mid(styleTextStartPos, styleTextEndPos - styleTextStartPos);
    QStringList properties = style_text.split(QChar(';'), QString::SkipEmptyParts);
    foreach(QString property_text, properties) {
        if (property_text.trimmed().isEmpty()) {
            continue;
        }

        QStringList name_values = property_text.split(QChar(':'), QString::SkipEmptyParts);
        CSSProperty *css_property = new CSSProperty();

        // Any badly formed CSS or stuff we don't "understand" like pre-processing we leave as is
        if (name_values.count() != 2) {
            css_property->name = property_text.trimmed();
            css_property->value = QString();
        } else {
            css_property->name = name_values.at(0).trimmed();
            css_property->value = name_values.at(1).trimmed();
        }

        new_properties.append(css_property);
    }
    return new_properties;
}

QString CSSInfo::formatCSSProperties(QList<CSSInfo::CSSProperty *> new_properties, bool multipleLineFormat, const int &selectorIndent)
{
    QString tab_spaces = QString(" ").repeated(TAB_SPACES_WIDTH + selectorIndent);

    if (new_properties.count() == 0) {
        if (multipleLineFormat) {
            return QString("\n%1")
                   .arg(QString(" ").repeated(selectorIndent));
        } else {
            return QString("");
        }
    } else {
        QStringList property_values;
        foreach(CSSInfo::CSSProperty * new_property, new_properties) {
            if (new_property->value.isNull()) {
                property_values.append(new_property->name);
            } else {
                property_values.append(QString("%1: %2").arg(new_property->name).arg(new_property->value));
            }
        }

        if (multipleLineFormat) {
            return QString("\n%1%2;\n%3")
                   .arg(tab_spaces)
                   .arg(property_values.join(";\n" % tab_spaces))
                   .arg(QString(" ").repeated(selectorIndent));
        } else {
            return QString(" %1; ").arg(property_values.join("; "));
        }
    }
}

bool CSSInfo::findInlineStyleBlock(const QString &text, const int &offset, int &styleStart, int &styleEnd)
{
    QRegularExpression inline_styles_search("<\\s*style\\s[^>]+>", QRegularExpression::CaseInsensitiveOption|QRegularExpression::InvertedGreedinessOption);
    int style_len = 0;
    styleEnd = -1;
    styleStart = -1;

    QRegularExpressionMatch match = inline_styles_search.match(text, offset);
    if (match.hasMatch()) {
        styleStart = match.capturedStart();
        style_len = match.capturedLength();
    }

    if (styleStart > 0) {
        styleStart += style_len;
        styleEnd = text.indexOf(QRegularExpression("<\\s*/\\s*style\\s*>", QRegularExpression::CaseInsensitiveOption), styleStart);

        if (styleEnd >= styleStart) {
            return true;
        }
    }

    return false;
}

void CSSInfo::parseCSSSelectors(const QString &text, const int &offsetLines, const int &offsetPos)
{
    QRegularExpression strip_attributes_regex("\\[[^\\]]*\\]");
    QRegularExpression strip_ids_regex("#[^\\s\\.]+");
    // QRegularExpression strip_non_name_chars_regex("[^A-Za-z0-9_\\-\\.:]+");
    QRegularExpression strip_non_name_chars_regex("[^\\w_\\-\\.:]+", QRegularExpression::UseUnicodePropertiesOption);
    QString search_text = replaceBlockComments(text);
    // CSS selectors can be in a myriad of formats... the class based selectors could be:
    //    .c1 / e1.c1 / e1.c1.c2 / e1[class~=c1] / e1#id1.c1 / e1.c1#id1 / .c1, .c2 / ...
    // Then the element based selectors could be:
    //    e1 / e1 > e2 / e1 e2 / e1 + e2 / e1[attribs...] / e1#id1 / e1, e2 / ...
    // Really needs a parser to do this properly, this will only handle the 90% scenarios.

    // Note: selector groups can be sepaparated by line feeds so you can not stop
    // at the beginning of line when searching for the start of a selector

    int pos = 0;
    int open_brace_pos = -1;
    int close_brace_pos = -1;

    while (true) {
        open_brace_pos = search_text.indexOf(QChar('{'), pos);

        if (open_brace_pos < 0) {
            break;
        }

        // Now search backwards until we get a line (or more)  containing text .
        bool have_text = false;
        pos = open_brace_pos - 1;

        while ((pos >= 0) && (!DELIMITERS.contains(search_text.at(pos)) || !have_text)) {
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
        foreach(QString match, matches) {
            CSSSelector *selector = new CSSSelector();
            selector->originalText = selector_text;
            selector->groupText = match.trimmed();
            selector->position = pos + offsetPos;
            selector->line = line + offsetLines;
            selector->isGroup = matches.length() > 1;
            selector->openingBracePos = open_brace_pos + offsetPos;
            selector->closingBracePos = close_brace_pos + offsetPos;
            // Need to parse our selector text to determine what sort of selector it contains.
            // First strip out any attributes and then identifiers
            match.replace(strip_attributes_regex, "");
            match.replace(strip_ids_regex, "");
            // Also replace any other characters like > or + not of interest
            match.replace(strip_non_name_chars_regex, QChar(' '));
            // Now break it down into the element components
            QStringList elements = match.trimmed().split(QChar(' '), QString::SkipEmptyParts);
            foreach(QString element, elements) {
                if (element.contains(QChar('.'))) {
                    QStringList parts = element.split('.');

                    if (!parts.at(0).isEmpty()) {
                        selector->elementNames.append(parts.at(0));
                    }

                    for (int i = 1; i < parts.length(); i++) {
                        selector->classNames.append(parts.at(i));
                    }
                } else {
                    selector->elementNames.append(element);
                }
            }
            m_CSSSelectors.append(selector);
        }
        pos = open_brace_pos + 1;
    }
}

QString CSSInfo::replaceBlockComments(const QString &text)
{
    // We take a copy of the text and remove all block comments from it.
    // However we must be careful to replace with spaces/keep line feeds
    // so that do not corrupt the position information used by the parser.
    QString new_text(text);
    QRegularExpression comment_search("/\\*.*\\*/", QRegularExpression::InvertedGreedinessOption|QRegularExpression::DotMatchesEverythingOption);
    int start = 0;
    int comment_index;

    while (true) {
        int comment_len = 0;
        comment_index = -1;
        QRegularExpressionMatch match = comment_search.match(new_text, start);
        if (match.hasMatch()) {
            comment_index = match.capturedStart();
            comment_len = match.capturedLength();
        }

        if (comment_index < 0) {
            break;
        }

        QString match_text = new_text.mid(comment_index, comment_len);
        match_text.replace(QRegularExpression("[^\r\n]"), QChar(' '));
        new_text.remove(comment_index, match_text.length());
        new_text.insert(comment_index, match_text);
        // Prepare for the next comment.
        start = comment_index + comment_len;

        if (start >= new_text.length() - 2) {
            break;
        }
    }

    return new_text;
}
