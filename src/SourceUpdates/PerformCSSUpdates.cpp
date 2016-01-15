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

#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "SourceUpdates/PerformCSSUpdates.h"

PerformCSSUpdates::PerformCSSUpdates(const QString &source, const QHash<QString, QString> &css_updates)
    :
    m_Source(source),
    m_CSSUpdates(css_updates)
{
}


QString PerformCSSUpdates::operator()()
{
    const QList<QString> &keys = m_CSSUpdates.keys();
    int num_keys = keys.count();
    if (num_keys == 0) return m_Source;

    // create a new updates dictionary to allow it to be used more effectively
    // lookup is by filename
    QHash<QString,QString> newupdates;
    foreach(QString key, keys) {
        const QString &filename = QFileInfo(key).fileName();
        newupdates[filename] = m_CSSUpdates[key];
    }
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
    QRegularExpressionMatch mo = reference.match(m_Source, start_index);
    do {
        for (int i = 1; i < reference.captureCount(); ++i) {
            if (mo.captured(i).trimmed().isEmpty()) {
                continue;
            }
            QString akey = mo.captured(i);
            const QString &filename = QFileInfo(akey).fileName();
            QString newpath = newupdates.value(filename, "");
            if (!newpath.isEmpty()) {
                m_Source.replace(mo.capturedStart(i), mo.capturedLength(i), newpath);
            }
        }
        start_index += mo.capturedLength();
        mo = reference.match(m_Source, start_index);
    } while (mo.hasMatch());

    return m_Source;
}
