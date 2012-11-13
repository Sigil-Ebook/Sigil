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
#include <QtCore/QRegExp>
#include <QtCore/QString>

#include "SourceUpdates/PerformCSSUpdates.h"

PerformCSSUpdates::PerformCSSUpdates(const QString &source, const QHash< QString, QString > &css_updates)
    :
    m_Source(source),
    m_CSSUpdates(css_updates)
{
}


QString PerformCSSUpdates::operator()()
{
    const QList< QString > &keys = m_CSSUpdates.keys();
    int num_keys = keys.count();

    for (int i = 0; i < num_keys; ++i) {
        const QString &key_path = keys.at(i);
        const QString &filename = QFileInfo(key_path).fileName();
        QString filename_regex_part =
            "[^\\(\\)\"']*/"
            + QRegExp::escape(filename) + "|"
            + QRegExp::escape(filename);
        QRegExp reference = QRegExp(
                                "(?:(?:src|background|background-image)\\s*:|@import)\\s*"
                                "[^;\\}\\(\"']*"
                                "(?:"
                                "url\\([\"']?(" + filename_regex_part + ")[\"']?\\)"
                                "|"
                                "[\"'](" + filename_regex_part + ")[\"']"
                                ")"
                                "[^;\\}]*"
                                "(?:;|\\})");
        int start_index = 0;

        do {
            m_Source.indexOf(reference, start_index);

            for (int i = 1; i < reference.captureCount(); ++i) {
                if (reference.cap(i).trimmed().isEmpty()) {
                    continue;
                }

                m_Source.replace(reference.pos(i), reference.cap(i).length(), m_CSSUpdates.value(key_path));
            }

            start_index += reference.matchedLength();
        } while (reference.matchedLength() != -1);
    }

    return m_Source;
}
