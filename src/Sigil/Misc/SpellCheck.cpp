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

#include <stdafx.h>
#include "SpellCheck.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QIODevice>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include "hunspell.hxx"
#include "Misc/SettingsStore.h"

static const QString USER_DICT = "user_dict.txt";

SpellCheck *SpellCheck::m_instance = 0;

SpellCheck *SpellCheck::instance()
{
    if (m_instance == 0) {
        m_instance = new SpellCheck();
    }

    return m_instance;
}

SpellCheck::~SpellCheck()
{
    if (m_hunspell) {
        delete m_hunspell;
        m_hunspell = 0;
    }

    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

QStringList SpellCheck::dictionaries()
{
    QStringList dicts;
    dicts = m_dictionaries.keys();
    dicts.sort();
    return dicts;
}

QString SpellCheck::currentDictionary()
{
    return m_dictionaryName;
}

bool SpellCheck::spell(const QString &word)
{
    if (!m_hunspell) {
        return true;
    }

    return m_hunspell->spell(m_codec->fromUnicode(word).constData()) != 0;
}

QStringList SpellCheck::suggest(const QString &word)
{
    if (!m_hunspell) {
        return QStringList();
    }

    QStringList suggestions;
    char **suggestedWords;

    int count = m_hunspell->suggest(&suggestedWords, m_codec->fromUnicode(word).constData());
    for (int i = 0; i < count; ++i) {
        suggestions << m_codec->toUnicode(suggestedWords[i]);
    }
    m_hunspell->free_list(&suggestedWords, count);

    return suggestions;
}

void SpellCheck::ignoreWord(const QString &word)
{
    if (!m_hunspell) {
        return;
    }

    m_hunspell->add(m_codec->fromUnicode(word).constData());
}

void SpellCheck::setDictionary(const QString &name, bool forceReplace)
{
    // See if we are already using a hunspell object for this language.
    if (!forceReplace && m_dictionaryName == name && m_hunspell) {
        return;
    }

    // Delete the current hunspell object.
    if (m_hunspell) {
        delete m_hunspell;
        m_hunspell = 0;
    }

    // Save the dictionary name for use later.
    m_dictionaryName = name;

    // If we don't have a dictionary we cannot continue.
    if (name.isEmpty() || !m_dictionaries.contains(name)) {
        return;
    }

    // Create a new hunspell object.
    QString aff = m_dictionaries.value(name) + ".aff";
    QString dic = m_dictionaries.value(name) + ".dic";
    m_hunspell = new Hunspell(aff.toLocal8Bit().constData(), dic.toLocal8Bit().constData());

    // Get the encoding for the text in the dictionary.
    QString encoding = "ISO8859-1";
    QFile affFile(aff);
    if (affFile.open(QIODevice::ReadOnly)) {
        QTextStream affStream(&affFile);
        QRegExp encDetector("^\\s*SET\\s+(.+)", Qt::CaseInsensitive);
        for (QString line = affStream.readLine(); !line.isNull(); line = affStream.readLine()) {
            if (line.contains(encDetector)) {
                encoding = encDetector.cap(1).trimmed();
                break;
            }
        }
        affFile.close();
    }
    m_codec = QTextCodec::codecForName(encoding.toLatin1());
    if (m_codec == 0) {
        m_codec = QTextCodec::codecForName("UTF-8");
    }

    // Load in the words from the user dictionary.
    foreach (QString word, userDictionaryWords()) {
        ignoreWord(word);
    }
}

void SpellCheck::addToUserDictionary(const QString &word)
{
    // Adding to the user dictionary also marks the word as a correct spelling.
    ignoreWord(word);

    if (!userDictionaryWords().contains(word)) {
        const QString userDict = userDictionaryName();
        QFile userDictFile(userDict);
        if (!userDictFile.exists()) {
            // Try to create the path in case it does not exist.
            QDir().mkpath(QFileInfo(userDict).absolutePath());
        }
        // Try to open the file to add the word.
        if (userDictFile.open(QIODevice::Append)) {
            QTextStream userDictStream(&userDictFile);
            userDictStream << word << "\n";
            userDictFile.close();
        }
    }
}

QStringList SpellCheck::userDictionaryWords()
{
    QStringList userWords;

    // Read each word from the user dictionary.
    QFile userDictFile(userDictionaryName());
    if (userDictFile.open(QIODevice::ReadOnly)) {
        QTextStream userDictStream(&userDictFile);
        for (QString line = userDictStream.readLine(); !line.isEmpty(); line = userDictStream.readLine()) {
            userWords << line;
        }
        userDictFile.close();
    }

    userWords.sort();
    return userWords;
}

void SpellCheck::replaceUserDictionaryWords(QStringList words)
{
    words.sort();

    // Delete everything from the user dictionary file.
    QFile userDictFile(userDictionaryName());
    if (userDictFile.open(QFile::WriteOnly | QFile::Truncate)) {
        userDictFile.close();
    }

    // Add all words to the user dictionary.
    foreach (QString word, words) {
        addToUserDictionary(word);
    }

    // Reload the dictionary so old user words are cleared.
    setDictionary(m_dictionaryName, true);
}

QString SpellCheck::dictionaryDirectory()
{
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/dictionaries";
}

SpellCheck::SpellCheck() :
    m_hunspell(0),
    m_codec(0)
{
    QStringList defaultDicts;
    defaultDicts << "de_DE-frami"
                 << "en_US"
                 << "es_MX"
                 << "fr-moderne";
    QStringList dictExts;
    dictExts << ".aff"
             << ".dic";

    const QString path = dictionaryDirectory();
    QDir dictDir(path);

    // Create the dictionary directory if it does not exist.
    if (!dictDir.exists()) {
        dictDir.mkpath(path);
    }

    // Write the default dictionaries to disk if they do not already exist.
    foreach (QString d, defaultDicts) {
        foreach (QString ext, dictExts) {
            QFile dFile(path + "/" + d + ext);
            if (!dFile.exists()) {
                if (dFile.open(QIODevice::WriteOnly)) {
                    QFile oFile(":/dict/" + d + ext);
                    if (oFile.open(QIODevice::ReadOnly)) {
                        dFile.write(oFile.readAll());
                    }
                }
            }
        }
    }

    // Find all dictionaries and add them to the avaliable list.
    if (dictDir.exists()) {
        QStringList filters;
        // Look for all .dic files.
        filters << "*.dic";
        dictDir.setNameFilters(filters);
        QStringList otherDicts = dictDir.entryList();
        foreach (QString ud, otherDicts) {
            const QFileInfo fileInfo(ud);
            const QString basename = fileInfo.baseName();
            const QString udPath = path + "/" + basename;
            // We only include the dictionary if it has a corresponding .aff.
            if (QFile(udPath + ".aff").exists()) {
                m_dictionaries.insert(basename, udPath);
            }
        }
    }

    // Load the dictionary the user has selected if one was saved.
    SettingsStore *store = SettingsStore::instance();
    setDictionary(store->dictionary());
}

QString SpellCheck::userDictionaryName() const
{
    return dictionaryDirectory() + "/" + USER_DICT;
}
