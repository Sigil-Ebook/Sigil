/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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

#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "Misc/Utility.h"
#include "SourceUpdates/PerformCSSUpdates.h"

static const QChar FORWARD_SLASH = QChar::fromLatin1('/');

PerformCSSUpdates::PerformCSSUpdates(const QString &source, const QHash<QString, QString> &css_updates, const QString &currentpath)
    :
    m_Source(source),
    m_CSSUpdates(css_updates),
    m_CurrentPath(currentpath)
{
}


QString PerformCSSUpdates::operator()()
{
    QString result(m_Source);
    QString origDir = QFileInfo(m_CurrentPath).dir().path();
    const QList<QString> &keys = m_CSSUpdates.keys();
    int num_keys = keys.count();
    if (num_keys == 0) return result;

    // Now parse the text once looking for keys and replacing them where needed
    QRegularExpression reference(
        "(?:(?:src|background|background-image|list-style|list-style-image|border-image|border-image-source|content|(?:-webkit-)?shape-outside)\\s*:|@import)\\s*"
        "("
        "[^;\\}]*"
        ")"
        "(?:;|\\})");

    QRegularExpression urls(
        "(?:"
        "url\\([\"']?([^\\(\\)\"']*)[\"']?\\)"
        "|"
        "[\"']([^\\(\\)\"']*)[\"']"
        ")");

    int start_index = 0;
    QRegularExpressionMatch mo = reference.match(result, start_index);
    // handle case if no initial match at all
    if (!mo.hasMatch()) return result;
    do {
        bool changes_made = false;
        for (int i = 1; i <= reference.captureCount(); ++i) {
            if (mo.captured(i).trimmed().isEmpty()) {
                continue;
            }
            // Check the captured property attribute string fragment for multiple urls
            int frag_start_index = 0;
            QString fragment = mo.captured(i);
            QRegularExpressionMatch frag_mo = urls.match(fragment, frag_start_index);
	    // only loop if at least one match was found
	    if (frag_mo.hasMatch()) {
                do {
                    for (int j = 1; j <= urls.captureCount(); ++j) {
                        if (frag_mo.captured(j).trimmed().isEmpty()) {
                            continue;
                        }
                        QString apath = Utility::URLDecodePath(frag_mo.captured(j));
                        QString search_key = QDir::cleanPath(origDir + FORWARD_SLASH + apath);
                        QString new_href;
                        if (m_CSSUpdates.contains(search_key)) {
                            new_href = m_CSSUpdates.value(search_key);
                        }
                        if (!new_href.isEmpty()) {
                            new_href = Utility::URLEncodePath(new_href);
                            // Replace the old url with the new one
                            fragment.replace(frag_mo.capturedStart(j), frag_mo.capturedLength(j), new_href);
                            changes_made = true;
                        }
    
                    }
                    frag_start_index += frag_mo.capturedLength();
                    frag_mo = urls.match(fragment, frag_start_index);
                } while (frag_mo.hasMatch());
	    }
            // Replace the original attribute string fragment with the new one
            if (changes_made) {
                result.replace(mo.capturedStart(i), mo.capturedLength(i), fragment);
            }

        }
        start_index += mo.capturedLength();
        mo = reference.match(result, start_index);
    } while (mo.hasMatch());

    return result;
}
