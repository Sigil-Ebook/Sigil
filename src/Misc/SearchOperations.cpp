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

#include <signal.h>

#include <QtCore/QtCore>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProgressDialog>

#include "BookManipulation/CleanSource.h"
#include "Misc/SearchOperations.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "PCRE/PCRECache.h"
#include "Misc/HTMLSpellCheck.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/TextResource.h"
#include "ViewEditors/Searchable.h"
#include "sigil_constants.h"

int SearchOperations::CountInFiles(const QString &search_regex,
                                   QList<Resource *> resources,
                                   SearchType search_type,
                                   bool check_spelling)
{
    QProgressDialog progress(QObject::tr("Counting occurrences.."), 0, 0, resources.count(), Utility::GetMainWindow());
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;
    progress.setValue(progress_value);
    // Count sequentially in order to see if occassional crashes are due to threading
    int count = 0;
    foreach(Resource * resource, resources) {
        progress.setValue(progress_value++);
        qApp->processEvents();
        count += CountInFile(search_regex, resource, search_type, check_spelling);
    }
    return count;
}


int SearchOperations::ReplaceInAllFIles(const QString &search_regex,
                                        const QString &replacement,
                                        QList<Resource *> resources,
                                        SearchType search_type)
{
    QProgressDialog progress(QObject::tr("Replacing search term..."), 0, 0, resources.count(), Utility::GetMainWindow());
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;
    progress.setValue(progress_value);
    // Replace sequentially in order to see if occassional crashes are due to threading
    int count = 0;
    foreach(Resource * resource, resources) {
        progress.setValue(progress_value++);
        qApp->processEvents();
        count += ReplaceInFile(search_regex, replacement, resource, search_type);
    }
    return count;
}


int SearchOperations::CountInFile(const QString &search_regex,
                                  Resource *resource,
                                  SearchType search_type,
                                  bool check_spelling)
{
    QReadLocker locker(&resource->GetLock());
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);

    if (html_resource) {
        return CountInHTMLFile(search_regex, html_resource, search_type, check_spelling);
    }

    TextResource *text_resource = qobject_cast<TextResource *>(resource);

    if (text_resource) {
        return CountInTextFile(search_regex, text_resource);
    }

    // We should never get here.
    return 0;
}



int SearchOperations::CountInHTMLFile(const QString &search_regex,
                                      HTMLResource *html_resource,
                                      SearchType search_type,
                                      bool check_spelling)
{
    if (search_type == SearchOperations::CodeViewSearch) {
        const QString &text = html_resource->GetText();

        if (check_spelling) {
            return HTMLSpellCheck::CountMisspelledWords(text, 0, text.count(), search_regex);
        } else {
            return PCRECache::instance()->getObject(search_regex)->getEveryMatchInfo(text).count();
        }
    }

    //TODO: BookViewSearch
    return 0;
}

int SearchOperations::CountInTextFile(const QString &search_regex, TextResource *text_resource)
{
    // TODO
    return 0;
}


int SearchOperations::ReplaceInFile(const QString &search_regex,
                                    const QString &replacement,
                                    Resource *resource,
                                    SearchType search_type)
{
    QWriteLocker locker(&resource->GetLock());
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);

    if (html_resource) {
        return ReplaceHTMLInFile(search_regex, replacement, html_resource, search_type);
    }

    TextResource *text_resource = qobject_cast<TextResource *>(resource);

    if (text_resource) {
        return ReplaceTextInFile(search_regex, replacement, text_resource);
    }

    // We should never get here.
    return 0;
}


int SearchOperations::ReplaceHTMLInFile(const QString &search_regex,
                                        const QString &replacement,
                                        HTMLResource *html_resource,
                                        SearchType search_type)
{
    SettingsStore ss;

    if (search_type == SearchOperations::CodeViewSearch) {
        int count;
        QString new_text;
        QString text = html_resource->GetText();
        std::tie(new_text, count) = PerformGlobalReplace(text, search_regex, replacement);
        html_resource->SetText(new_text);
        return count;
    }

    //TODO: BookViewSearch
    return 0;
}


int SearchOperations::ReplaceTextInFile(const QString &search_regex,
                                        const QString &replacement,
                                        TextResource *text_resource)
{
    // TODO
    return 0;
}


std::tuple<QString, int> SearchOperations::PerformGlobalReplace(const QString &text,
        const QString &search_regex,
        const QString &replacement)
{
    QString new_text = text;
    int count = 0;
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    QList<SPCRE::MatchInfo> match_info = spcre->getEveryMatchInfo(text);

    for (int i =  match_info.count() - 1; i >= 0; i--) {
        QString match_segement = Utility::Substring(match_info.at(i).offset.first, match_info.at(i).offset.second, new_text);
        QString replacement_text;

        if (spcre->replaceText(match_segement, match_info.at(i).capture_groups_offsets, replacement, replacement_text)) {
            new_text.replace(match_info.at(i).offset.first, match_info.at(i).offset.second - match_info.at(i).offset.first, replacement_text);
            count++;
        }
    }

    return std::make_tuple(new_text, count);
}


//std::tuple<QString, int> SearchOperations::PerformHTMLSpellCheckReplace(const QString &text,
//        const QString &search_regex,
//        const QString &replacement)
//{
//    QString new_text = text;
//    int count = 0;
//    int offset = 0;
//    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
//    QList<HTMLSpellCheck::MisspelledWord> check_spelling = HTMLSpellCheck::GetMisspelledWords(text, 0, text.count(), search_regex);
//    foreach(HTMLSpellCheck::MisspelledWord misspelled_word, check_spelling) {
//        SPCRE::MatchInfo match_info = spcre->getFirstMatchInfo(misspelled_word.text);

//        if (match_info.offset.first != -1) {
//            QString replacement_text;

//            if (spcre->replaceText(Utility::Substring(match_info.offset.first, match_info.offset.second, misspelled_word.text), match_info.capture_groups_offsets, replacement, replacement_text)) {
//                new_text.replace(offset + misspelled_word.offset + match_info.offset.first, match_info.offset.second - match_info.offset.first, replacement_text);
//                offset += replacement_text.length() - (match_info.offset.second - match_info.offset.first);
//                count++;
//            }
//        }
//    }
//    return std::make_tuple(new_text, count);
//}


void SearchOperations::Accumulate(int &first, const int &second)
{
    first += second;
}
