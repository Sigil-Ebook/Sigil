/************************************************************************
**
**  Copyright (C) 2016-2021 Kevin B. Hendricks, Stratford, ON Canada
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

#include "EmbedPython/EmbeddedPython.h"

#include <QString>
#include <QDebug>
#include <QRegularExpression>
#include "Misc/Utility.h"
#include "Parsers/CSSInfo.h"

const int TAB_SPACES_WIDTH = 4;
const QString LINE_MARKER("[SIGIL_NEWLINE]");
static const QString DELIMITERS = "}{;";

// Note: CSSProperties and CSSSelectors are simple struct that this code
// created with new and so need to be manually cleaned up to prevent
// large memory leaks

CSSInfo::CSSInfo(const QString &text, int offset)
    : m_source(text)
{
    m_posoffset = offset;
    parseStyles(text, m_posoffset);
}


// Need to manually clean up the Selector List since allcated with new
CSSInfo::~CSSInfo()
{
    foreach(CSSSelector * sp, m_CSSSelectors) {
        if (sp) delete sp;
    }
    m_CSSSelectors.clear();
}


QList<CSSInfo::CSSSelector *> CSSInfo::getAllSelectors()
{
    QList<CSSInfo::CSSSelector *> selectors;
    foreach(CSSInfo::CSSSelector * cssSelector, m_CSSSelectors) {
        selectors.append(cssSelector);
    }
    return selectors;
}


QList<CSSInfo::CSSSelector *> CSSInfo::getClassSelectors(const QString filterClassName)
{
    QList<CSSInfo::CSSSelector *> selectors;
    foreach(CSSInfo::CSSSelector * cssSelector, m_CSSSelectors) {
        if (!cssSelector->className.isEmpty()) {
            if (filterClassName.isEmpty() || cssSelector->className == filterClassName) {
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
                if (cssSelector->elementName.isEmpty()) {
                    return cssSelector;
                }
                if (cssSelector->elementName == elementName) {
                    // Doublecheck that the full element.class is actually in the text
                    // to avoid, e.g.,  div class="test" matching p.test + div
                    if (cssSelector->text.contains(elementName % "." % className)) {
                        return cssSelector;
                    }
                }
            }
        }
    } else {
        // try match on element name alone
        foreach(CSSInfo::CSSSelector * cssSelector, m_CSSSelectors) {
            if ((cssSelector->elementName == elementName) && (cssSelector->className.isEmpty())) {
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
                if (cssSelector->elementName.isEmpty()) {
                    matches.append(cssSelector);;
                }
                if (cssSelector->elementName == elementName) {
                    matches.append(cssSelector);
                }
            }
        }
    } else {
        // try match on element name alone
        foreach(CSSInfo::CSSSelector * cssSelector, m_CSSSelectors) {
            if ((cssSelector->elementName == elementName) && (cssSelector->className.isEmpty())) {
                matches.append(cssSelector);
            }
        }
    }
    return matches;
}


QStringList CSSInfo::getAllPropertyValues(QString property)
{
    QStringList property_values;
    bool inselector = false;
    bool get_value = false;
    int i = 0;
    while(i < m_csstokens.size()) {
        CSSParser::token atoken = m_csstokens[i];
        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) inselector = true;
        if (atoken.type == CSSParser::SEL_END && !atoken.data.startsWith('@')) inselector = false;
        if (atoken.type == CSSParser::PROPERTY && inselector) {
            get_value = (atoken.data == property) || property.isEmpty();
        }
        if (atoken.type == CSSParser::VALUE && inselector) {
            if (get_value) {
                property_values << atoken.data;
                get_value = false;
            }
        }
        i++;
    }
    return property_values;
}


QString CSSInfo::getReformattedCSSText(bool multipleLineFormat)
{
    QString csstext(m_source);

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
}


QString CSSInfo::removeMatchingSelectors(QList<CSSSelector *> cssSelectors)
{
    // First try to find a CSS selector currently parsed that matches each of the selectors supplied.
    QList<CSSSelector *> remove_selectors;
    foreach(CSSSelector * css_selector, cssSelectors) {
        foreach(CSSSelector * match_selector, m_CSSSelectors) {
            if ((match_selector->pos == css_selector->pos) &&
                (match_selector->text == css_selector->text)) {
                remove_selectors.append(match_selector);
            }
        }
    }

    // If no matches found, return a null string to caller
    if (remove_selectors.isEmpty()) {
        return QString();
    }

    // Sort the selectors by pos ascending.
    std::sort(remove_selectors.begin(), remove_selectors.end(), dereferencedLessThan<CSSSelector>);

    QVector<CSSParser::token> new_csstokens;

    int i = 0;
    while(i < m_csstokens.size()) {
        CSSParser::token atoken = m_csstokens[i];
        bool store_it = true;
        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) {
            // we have a selector
            QStringList sels = CSSParser::splitGroupSelector(atoken.data);

            // now walk though the remove selector list looking
            // for matching selector by position (unique key) and text and if matching
            // remove this selector
            foreach(CSSSelector * css_selector, remove_selectors) {
                if (css_selector->pos < atoken.pos) continue;
                if (css_selector->pos == atoken.pos) {
                    int found = -1;
                    for (int i = 0; i < sels.size(); i++) {
                        if (css_selector->text == sels.at(i)) {
                            found = i;
                            break;
                        }
                    }
                    if (found != -1) sels.removeAt(found);
                }
                if (css_selector->pos > atoken.pos) break;
            }
            if (!sels.isEmpty()) {
                // recreate this token
                if (sels.size() == 1) {
                    atoken.data = sels.at(0);
                } else {
                    atoken.data = sels.join(',');
                }
            } else {
                // skip this SEL_START all of the way to the SEL_END
                store_it = false;
                while(atoken.type != CSSParser::SEL_END) {
                    i++;
                    if (i >=  m_csstokens.size()) break;
                    atoken = m_csstokens[i];
                }
            }
        }
        if (store_it) new_csstokens.push_back(atoken);
        i++;
    }
    CSSParser cp;
    cp.set_level("CSS3.0");
    cp.set_csstokens(new_csstokens);
    QString new_text = cp.serialize_css(false);
    if (new_text.isEmpty()) new_text = "/* CSS */\n";

    // IMPORTANT: After removing any selectors, users *must*
    // Initialize a new CSSInfo object to work on the new css text.
    // This CSSInfo is now obsolete.
    return new_text;
}


void CSSInfo::parseStyles(const QString &text, int offset)
{
    CSSParser cp;
    cp.set_level("CSS3.0"); // most permissive
    cp.parse_css(text);

    // report any parser errors (should we abort?)
    QVector<QString> errors = cp.get_parse_errors();
    for(int i = 0; i < errors.size(); i++) {
        qDebug() << "  CSS Parser Error: " << errors[i] << "\n";
    }

    // now store the sequence of parsed tokens
    CSSParser::token atoken = cp.get_next_token();
    while(atoken.type != CSSParser::CSS_END)
    {
        CSSParser::token temp;
        temp.pos = atoken.pos + offset;
        temp.line = atoken.line;
        temp.type = atoken.type;
        temp.data = atoken.data;
        m_csstokens.append(temp);
        atoken = cp.get_next_token();
    }
    CSSParser::token temp;
    temp.pos = -1;
    temp.line = -1;
    temp.type = CSSParser::CSS_END;
    temp.data = "";
    m_csstokens.append(temp);  // end marker token

    generateSelectorsList();
}


void CSSInfo::generateSelectorsList()
{
    // now walk the sequence of previously parsed tokens
    int i = 0;
    while(i < m_csstokens.size()) {
        CSSParser::token atoken = m_csstokens[i];

        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) {
            QStringList sels = CSSParser::splitGroupSelector(atoken.data);

            foreach(QString asel, sels) {

                CSSSelector *selector = new CSSSelector();
                selector->text = asel;
                selector->pos = atoken.pos;

                // if a pure class selector or pure element selector
                bool uses_pseudoclasses = asel.contains(':');
                bool uses_combinator = asel.contains(' ') || asel.contains('>') ||
                                       asel.contains('~') || asel.contains('+');

                if (!uses_combinator && !uses_pseudoclasses) {
                    if (asel.contains('.')) {
                        QStringList parts = asel.split('.');
                        if (!parts.at(0).isEmpty()) selector->elementName = parts.at(0);
                        if (!parts.at(1).isEmpty()) selector->className = parts.at(1);
                    } else {
                        selector->elementName = asel;
                    }
                }
                m_CSSSelectors.append(selector);
            }
        }
        i++;
    }
}
