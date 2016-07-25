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
        "(?:(?:src|background|background-image|list-style|list-style-image|border-image|border-image-source|content)\\s*:|@import)\\s*"
        "[^;\\}\\(\"']*"
        "(?:"
        "url\\([\"']?([^\\(\\)\"']*)[\"']?\\)"
        "|"
        "[\"']([^\\(\\)\"']*)[\"']"
        ")"
        "[^;\\}]*"
        "(?:;|\\})");
    int start_index = 0;
    QRegularExpressionMatch mo = reference.match(result, start_index);
    do {
        for (int i = 1; i < reference.captureCount(); ++i) {
            if (mo.captured(i).trimmed().isEmpty()) {
                continue;
            }
            QString apath = Utility::URLDecodePath(mo.captured(i));
            QString search_key = QDir::cleanPath(origDir + FORWARD_SLASH + apath);
            QString new_href;
            if (m_CSSUpdates.contains(search_key)) {
                new_href = m_CSSUpdates.value(search_key);
            }
            if (!new_href.isEmpty()) {
                new_href = Utility::URLEncodePath(new_href);
                result.replace(mo.capturedStart(i), mo.capturedLength(i), new_href);
            }

        }
        start_index += mo.capturedLength();
        mo = reference.match(result, start_index);
    } while (mo.hasMatch());

    return result;
}
