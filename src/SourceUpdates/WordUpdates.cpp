/************************************************************************
**
**  Copyright (C) 2015-2021 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2013      Dave Heiland
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

#include <functional>

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

#include "Misc/HTMLSpellCheckML.h"
#include "ResourceObjects/HTMLResource.h"
#include "SourceUpdates/WordUpdates.h"

void WordUpdates::UpdateWordInAllFiles(const QList<HTMLResource *> &html_resources,
                                       const QString& default_lang,
                                       const QString& old_word,
                                       const QString& new_word)
{
    QtConcurrent::blockingMap(html_resources, std::bind(UpdateWordsInOneFile, std::placeholders::_1, default_lang, old_word, new_word));
}

void WordUpdates::UpdateWordsInOneFile(HTMLResource *html_resource,
                                       const QString& default_lang,
                                       const QString& old_word,
                                       const QString& new_word)
{
    // qDebug() << "UpdateWordsInOneFile " << html_resource->Filename() << old_word << new_word;
    Q_ASSERT(html_resource);
    QWriteLocker locker(&html_resource->GetLock());
    QString text = html_resource->GetText();
    QList<HTMLSpellCheckML::AWord> words = HTMLSpellCheckML::GetWords(text, default_lang);

    // Change in reverse to preserve location information
    for (int i = words.count() - 1; i >= 0; i--) {
        HTMLSpellCheckML::AWord word = words[i];
        if (word.text != old_word) {
            continue;
        }
        text.replace(word.offset, word.length, HTMLSpellCheckML::textOf(new_word));
    }
    html_resource->SetText(text);
}
