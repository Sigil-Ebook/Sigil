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

#include <hunspell.hxx>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QIODevice>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

#include "Misc/SpellCheck.h"
#include "Misc/SettingsStore.h"

#ifdef Q_WS_X11
#include <stdlib.h>
#endif

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
    loadDictionaryNames();
    QStringList dicts;
    dicts = m_dictionaries.keys();
    dicts.sort();
    return dicts;
}

QString SpellCheck::currentDictionary() const
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

    // Dictionary files to use.
    QString aff = QString("%1%2.aff").arg(m_dictionaries.value(name)).arg(name);
    QString dic = QString("%1%2.dic").arg(m_dictionaries.value(name)).arg(name);
    QString hyph_dic = QString("%1hyph_%2.dic").arg(m_dictionaries.value(name)).arg(name);
    // Create a new hunspell object.
    m_hunspell = new Hunspell(aff.toLocal8Bit().constData(), dic.toLocal8Bit().constData());
    // Load the hyphenation dictionary if it exists.
    if (QFile::exists(hyph_dic)) {
        m_hunspell->add_dic(hyph_dic.toLocal8Bit().constData());
    }

    // Get the encoding for the text in the dictionary.
    m_codec = QTextCodec::codecForName(m_hunspell->get_dic_encoding());
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
        const QString userDict = currentUserDictionaryFile();
        QFile userDictFile(userDict);
        if (!userDictFile.exists()) {
            // Try to create the path in case it does not exist.
            QDir().mkpath(QFileInfo(userDict).absolutePath());
        }
        // Try to open the file to add the word.
        if (userDictFile.open(QIODevice::Append)) {
            QTextStream userDictStream(&userDictFile);
            userDictStream.setCodec("UTF-8");
            userDictStream << word << "\n";
            userDictFile.close();
        }
    }
}

QStringList SpellCheck::userDictionaryWords()
{
    QStringList userWords;

    // Read each word from the user dictionary.
    QFile userDictFile(currentUserDictionaryFile());
    if (userDictFile.open(QIODevice::ReadOnly)) {
        QTextStream userDictStream(&userDictFile);
        userDictStream.setCodec("UTF-8");
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
    QFile userDictFile(currentUserDictionaryFile());
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

SpellCheck::SpellCheck() :
    m_hunspell(0),
    m_codec(0)
{

    loadDictionaryNames();

    // Create the user dictionary word list directiory if neccessary and
    // create the configured file if necessary.
    const QString user_directory = userDictionaryDirectory();
    QDir userDir(user_directory);
    if (!userDir.exists()) {
        userDir.mkpath(user_directory);
    }
    QFile userFile(currentUserDictionaryFile());
    if (!userFile.exists()) {
        if (userFile.open(QIODevice::WriteOnly)) {
            userFile.close();
        }
    }

    // Load the dictionary the user has selected if one was saved.
    SettingsStore settings;
    setDictionary(settings.dictionary());
}


void SpellCheck::loadDictionaryNames()
{
    QStringList dictExts;
    dictExts << ".aff"
             << ".dic";

    m_dictionaries.clear();

    const QString user_directory = dictionaryDirectory();
    QDir userDir(user_directory);
    // Create the user dictionary directory if it does not exist.
    if (!userDir.exists()) {
        userDir.mkpath(user_directory);
    }

    // Paths for each dictionary location.
    QStringList paths;
#ifdef Q_WS_MAC
    paths << QCoreApplication::applicationDirPath() + "/../hunspell_dictionaries";
#endif
#if defined( Q_WS_WIN ) || defined( Q_WS_X11 )
    paths << QCoreApplication::applicationDirPath() + "/hunspell_dictionaries";
#endif
#ifdef Q_WS_X11
    // The user can specify an env variable that points to the dictionaries.
    const QString env_dic_location = QString(getenv("SIGIL_DICTIONARIES"));
    if (!env_dic_location.isEmpty()) {
        paths << env_dic_location;
    }
    // Possible location if the user installed from source.
    // This really should be changed to be passed the install prefix given to
    // cmake instead of guessing based upon the executable path.
    paths << QCoreApplication::applicationDirPath() + "/../share/" + QCoreApplication::applicationName().toLower() + "/hunspell_dictionaries/";
#endif
    // Add the user dictionary directory last because anything in here
    // will override installation supplied dictionaries.
    paths << user_directory;

    foreach (QString path, paths) {
        // Find all dictionaries and add them to the avaliable list.
        QDir dictDir(path);
        if (dictDir.exists()) {
            QStringList filters;
            // Look for all .dic files.
            filters << "*.dic";
            dictDir.setNameFilters(filters);
            QStringList otherDicts = dictDir.entryList();
            foreach (QString ud, otherDicts) {
                const QFileInfo fileInfo(ud);
                const QString basename = fileInfo.baseName();
                const QString udPath = path + "/";
                // We only include the dictionary if it has a corresponding .aff.
                if (QFile(udPath + basename + ".aff").exists()) {
                    m_dictionaries.insert(basename, udPath);
                }
            }
        }
    }
}

QString SpellCheck::dictionaryDirectory()
{
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/hunspell_dictionaries";
}

QString SpellCheck::userDictionaryDirectory()
{
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/user_dictionaries";
}

QString SpellCheck::currentUserDictionaryFile()
{
    SettingsStore settings;
    return userDictionaryDirectory() + "/" + settings.userDictionaryName();
}
