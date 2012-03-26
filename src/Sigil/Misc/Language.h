/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>

class QString;
class QStringList;

/**
 * Singleton.
 *
 * Language routines
 */
class Language
{
    Q_DECLARE_TR_FUNCTIONS(Language)

public:
    static Language *instance();

    QString GetLanguageName(QString language_code);
    QString GetLanguageCode(QString language_name);
    QStringList GetSortedPrimaryLanguageNames();

private:
    Language();

    void SetLanguageMap();

    // Use hash since order is not important (sort later)
    QHash<QString, QString> m_languageCodeMap;
    QHash<QString, QString> m_languageNameMap;

    QStringList m_sortedPrimaryLanguageNames;

    static Language *m_instance;
};

#endif // LANGUAGE_H
