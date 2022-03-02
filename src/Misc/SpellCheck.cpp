/************************************************************************
**
**  Copyright (C) 2015-2021 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2011      John Schember <john@nachtimwald.com>
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

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTextCodec>
#include <QTextStream>
#include <QUrl>
#include <QApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

#include "Misc/HTMLSpellCheckML.h"
#include "Misc/SpellCheck.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"

#define DBG if(0)

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
# include <stdlib.h>
#endif

SpellCheck *SpellCheck::m_instance = 0;

SpellCheck *SpellCheck::instance()
{
    if (m_instance == 0) {
        m_instance = new SpellCheck();
    }

    return m_instance;
}

SpellCheck::SpellCheck()
{
    DBG qDebug() << "In SpellCheck Constructor";
    m_primary.handle = NULL;
    m_secondary.handle = NULL;

    // There is a considerable lag involved in loading the Spellcheck dictionaries
    QApplication::setOverrideCursor(Qt::WaitCursor);
    loadDictionaryNames();
    // Create the user dictionary word list directiory if necessary.
    const QString user_directory = userDictionaryDirectory();
    QDir userDir(user_directory);

    if (!userDir.exists()) {
        userDir.mkpath(user_directory);
    }

    // Create the configured file if necessary.
    QFile userFile(currentUserDictionaryFile());

    if (!userFile.exists()) {
        if (userFile.open(QIODevice::WriteOnly)) {
            userFile.close();
        }
    }

    QApplication::restoreOverrideCursor();

    UpdateLangCodeToDictMapping();

    // Load the dictionary the user has selected
    // now open primary and secondary dictionaries
    SettingsStore settings;
    loadDictionary(settings.dictionary());
    if (!settings.secondary_dictionary().isEmpty()) {
        loadDictionary(settings.secondary_dictionary());
    }
}

void SpellCheck::UpdateLangCodeToDictMapping()
{
    DBG qDebug() << "In UpdateLangCodeToDictMapping";
    m_langcode2dict.clear();

    // create language code to dictionary name mapping
    foreach(QString dname, m_dictionaries.keys()) {
        QString lc = dname;
        lc.replace("_","-");
        m_langcode2dict[lc] = dname;
        if (lc.length() > 3) {
            lc = lc.mid(0,2);
            m_langcode2dict[lc] = dname;
        }
    }

    // make sure 2 letter mapping equivalent is properly set
    // for primary and secondary dictionaries
    // Note: must be done last to overwrite any earlier values
    SettingsStore settings;
    QString cd = settings.secondary_dictionary();
    cd.replace("_","-");
    if (!cd.isEmpty() && (cd.length() > 3)) {
        m_langcode2dict[cd.mid(0,2)] = settings.secondary_dictionary();
    }
    cd = settings.dictionary();
    cd.replace("_","-");
    if (!cd.isEmpty() && (cd.length() > 3)) {
        m_langcode2dict[cd.mid(0,2)] = settings.dictionary();
    }
}

void SpellCheck::UnloadDictionary(const QString &dname)
{
    DBG qDebug() << "In UnloadDictionary";
    QMutexLocker locker(&mutex);
    if (m_opendicts.contains(dname)) {
        HDictionary hdic = m_opendicts[dname];
        if (hdic.handle) {
            delete hdic.handle;
        }
        m_opendicts.remove(dname);
    }
}

void SpellCheck::UnloadAllDictionaries()
{
    DBG qDebug() << "In UnloadAllDictionaries";
    foreach(QString name, m_opendicts.keys()) {
        UnloadDictionary(name);
    }
}

SpellCheck::~SpellCheck()
{
    DBG qDebug() << "In SpellCheck destructor";
    UnloadAllDictionaries();

    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

QStringList SpellCheck::userDictionaries()
{
    DBG qDebug() << "In userDictionaries";
    // Load the list of user dictionaries.
    QDir userDictDir(userDictionaryDirectory());
    QStringList user_dicts = userDictDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    user_dicts.sort();
    return user_dicts;
}

QStringList SpellCheck::dictionaries()
{
    DBG qDebug() << "In dictionaries";
    loadDictionaryNames();
    QStringList dicts;
    dicts = m_dictionaries.keys();
    dicts.sort();
    return dicts;
}

QString SpellCheck::currentPrimaryDictionary() const
{
    DBG qDebug() << "In currentPrimaryDictionary";
    SettingsStore settings;
    return settings.dictionary();
}

bool SpellCheck::spell(const QString &word)
{
    DBG qDebug() << "In spell";
    QString dname = m_langcode2dict.value(HTMLSpellCheckML::langOf(word), "");

    // if no dictionary exists for this language treat it as correct
    if (dname.isEmpty()) return true;

    // if a dictionary exists but is not open yet, open it first
    if (!m_opendicts.contains(dname)) {
        loadDictionary(dname);
    }
    if (!m_opendicts.contains(dname)) return true;
    HDictionary hdic = m_opendicts[dname];
    Q_ASSERT(hdic.codec != nullptr);
    Q_ASSERT(hdic.handle != nullptr);
    bool res = hdic.handle->spell(hdic.codec->fromUnicode(Utility::getSpellingSafeText(HTMLSpellCheckML::textOf(word))).constData()) != 0;
    res = res || isIgnored(HTMLSpellCheckML::textOf(word));
    return res;
}


// Speed here is very important as it is invoked by the XHTMLHighlighter2 code
// and this is the limiting factor
// spell check word without langcode info in Primary and Secondary Dictionaries
bool SpellCheck::spellPS(const QString &word)
{
    if (!m_primary.handle) return true;
    if(m_ignoredWords.contains(word)) return true;
    bool res = m_primary.handle->spell(m_primary.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData()) != 0;
    if (res) return true;
    if (!m_secondary.handle) return false;
    return m_secondary.handle->spell(m_secondary.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData()) != 0;
}


QStringList SpellCheck::suggest(const QString &word)
{
    DBG qDebug() << "In suggest";
    QStringList suggestions;
    char **suggestedWords;
    QString dname = m_langcode2dict.value(HTMLSpellCheckML::langOf(word), "");
    if (dname.isEmpty()) return suggestions;
    if (!m_opendicts.contains(dname)) return suggestions;
    HDictionary hdic = m_opendicts[dname];
    Q_ASSERT(hdic.codec != nullptr);
    Q_ASSERT(hdic.handle != nullptr);
    int count = hdic.handle->suggest(&suggestedWords, hdic.codec->fromUnicode(Utility::getSpellingSafeText(HTMLSpellCheckML::textOf(word))).constData());
    bool possible_end_of_sentence = word.endsWith('.') && (word.count(".") == 1);
    for (int i = 0; i < count; ++i) {
        QString suggested_word = hdic.codec->toUnicode(suggestedWords[i]);
        if (possible_end_of_sentence && !suggested_word.endsWith('.')) suggestions << suggested_word + ".";
        suggestions << suggested_word;
    }

    hdic.handle->free_list(&suggestedWords, count);
    return suggestions;
}


// suggesttions for word without langcode using Primary and Secondary Dictionaries
QStringList SpellCheck::suggestPS(const QString &word)
{
    QStringList suggestions;
    char **suggestedWords;
    char **suggestedWords2;
    if (!m_primary.handle) return suggestions;
    bool possible_end_of_sentence = word.endsWith('.') && (word.count(".") == 1);
    int count = m_primary.handle->suggest(&suggestedWords, m_primary.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());
    int limit = count;
    if (limit > 4) limit = 4;
    for (int i = 0; i < limit; ++i) {
        QString suggested_word = m_primary.codec->toUnicode(suggestedWords[i]);
        if (possible_end_of_sentence && !suggested_word.endsWith('.')) suggested_word.append('.');
        suggestions << suggested_word;
    }
    m_primary.handle->free_list(&suggestedWords, count);
    if (!m_secondary.handle) return suggestions; 
    count = m_secondary.handle->suggest(&suggestedWords2, m_secondary.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());
    limit = count;
    if (limit > 4) limit = 4;
    for (int i = 0; i < limit; ++i) {
        QString suggested_word = m_secondary.codec->toUnicode(suggestedWords2[i]);
        if (possible_end_of_sentence && !suggested_word.endsWith('.')) suggested_word.append('.');
        suggestions << suggested_word;
    }
    m_secondary.handle->free_list(&suggestedWords2, count);
    return suggestions;
}


void SpellCheck::clearIgnoredWords()
{
    DBG qDebug() << "In clearIgnoredWords";
    m_ignoredWords.clear();
}


void SpellCheck::ignoreWord(const QString &word)
{
    DBG qDebug() << "In ignoreWord";
    m_ignoredWords.insert(word);
}


bool SpellCheck::isIgnored(const QString &word) {
    DBG qDebug() << "In isIgnored";
    return m_ignoredWords.contains(word);
}


void SpellCheck::addWordToDictionary(const QString &word, const QString &dname)
{
    DBG qDebug() << "In addWordToDictionary";
    if (dname.isEmpty()) return;
    if (m_opendicts.contains(dname)) {
        HDictionary hdic = m_opendicts[dname];
        hdic.handle->add(hdic.codec->fromUnicode(Utility::getSpellingSafeText(HTMLSpellCheckML::textOf(word))).constData());
    }
}


void SpellCheck::loadDictionary(const QString &dname)
{
    DBG qDebug() << "In loadDictionary: " << dname;
    QMutexLocker locker(&mutex);
    // If we don't have a dictionary we cannot continue.
    if (dname.isEmpty() || !m_dictionaries.contains(dname)) {
        qDebug() << "attempted to load a non-existent dictionary: " << dname;
        return;
    }

    // Dictionary files to use.
    QString aff = QString("%1%2.aff").arg(m_dictionaries.value(dname)).arg(dname);
    QString dic = QString("%1%2.dic").arg(m_dictionaries.value(dname)).arg(dname);
    QString dic_delta = QString("%1/%2.dic_delta").arg(dictionaryDirectory()).arg(dname);
    QString alt_dic_delta = QString("%1%2.dic_delta").arg(m_dictionaries.value(dname)).arg(dname);
    // qDebug() << dic_delta;
    // qDebug() << alt_dic_delta;

    // Create a new hunspell object.
    HDictionary hdic;
    hdic.name = dname;
    hdic.handle = new Hunspell(aff.toLocal8Bit().constData(), dic.toLocal8Bit().constData());
    if (!hdic.handle) {
        qDebug() << "failed to load new Hunspell dictionary " << dname;
        return;
    }

    // Get the encoding for the text in the dictionary.
    hdic.codec = QTextCodec::codecForName(hdic.handle->get_dic_encoding());
    if (hdic.codec == nullptr) {
        hdic.codec = QTextCodec::codecForName("UTF-8");
    }
    if (!hdic.codec) {
        qDebug() << "failed to load codec " << dname;
        return;
    }

    // Get the extra wordchars used for tokenization
    hdic.wordchars = hdic.codec->toUnicode(hdic.handle->get_wordchars());

    // register it as an open dictionary
    m_opendicts[dname] = hdic;

    // check for appropriate .dic_delta file and add it
    // check in user prefs hunspell_dictionaries first
    // so that user's version is given preference over 
    // any system version
    QStringList deltaWords;
    if (QFile(dic_delta).exists()) {
        dicDeltaWords(dic_delta, deltaWords);
    } else if (QFile(alt_dic_delta).exists()) {
        dicDeltaWords(alt_dic_delta, deltaWords);
    }
    foreach(QString word, deltaWords){
        addWordToDictionary(word, dname);
    }

    // add UserDictionary words to the Primary Dictionary only
    if (dname == currentPrimaryDictionary()) {
        // Load in the words from the user dictionaries.
        foreach(QString word, allUserDictionaryWords()) {
            addWordToDictionary(word, dname);
        }
    }

    SettingsStore settings;
    // store the primary and secondary dictionary info for speed
    if (dname == settings.dictionary()) {
        m_primary = hdic;
    }
    else if (dname == settings.secondary_dictionary()) {
        m_secondary = hdic;
    }
    return;
}


void SpellCheck::setDictionary(const QString &dname, bool forceReplace)
{
    DBG qDebug() << "In setDictionary " << dname;
    // See if we are already using a hunspell object for this language.
    if (!forceReplace && m_opendicts.contains(dname)) {
        return;
    }

    UnloadDictionary(dname);
    loadDictionary(dname);
}


QString SpellCheck::getWordChars(const QString &lang)
{
    DBG qDebug() << "In getWordChars";
    QString dname;
    if (lang.isEmpty()) { 
        dname = currentPrimaryDictionary();
    } else {
        dname = m_langcode2dict.value(lang, "");
    }

    if (dname.isEmpty()) return "";

    // if a dictionary exists but is not open yet, open it first
    if (!m_opendicts.contains(dname)) {
        loadDictionary(dname);
    }
    if (!m_opendicts.contains(dname)) return "";
    HDictionary hdic = m_opendicts[dname];
    Q_ASSERT(hdic.codec != nullptr);
    Q_ASSERT(hdic.handle != nullptr);
    return hdic.wordchars;
}


void SpellCheck::addToUserDictionary(const QString &word, QString dict_name)
{
    DBG qDebug() << "In AddToUserDictionary";
    // Adding to the user dictionary also marks the word as a correct spelling.
    if (word.isEmpty()) {
        return;
    }

    SettingsStore settings;
    if (dict_name.isEmpty()) {
        dict_name = settings.defaultUserDictionary();
    }

    // Add the word only if the dictionary is enabled
    if (settings.enabledUserDictionaries().contains(dict_name)) {
        addWordToDictionary(word, currentPrimaryDictionary());
    }

    if (!userDictionaryWords(dict_name).contains(word)) {
        const QString userDict = userDictionaryFile(dict_name);
        QFile userDictFile(userDict);

        if (!userDictFile.exists()) {
            // Try to create the path in case it does not exist.
            QDir().mkpath(QFileInfo(userDict).absolutePath());
        }

        // Try to open the file to add the word.
        if (userDictFile.open(QIODevice::Append)) {
            QTextStream userDictStream(&userDictFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            userDictStream.setCodec("UTF-8");
#endif
            userDictStream << word << "\n";
            userDictFile.close();
        }
    }
}

QStringList SpellCheck::allUserDictionaryWords()
{
    DBG qDebug() << "In allUserDictionaryWords";
    QStringList userWords;
    SettingsStore settings;
    foreach (QString dict_name, settings.enabledUserDictionaries()) {
        userWords.append(userDictionaryWords(dict_name));
    }
    return userWords;
}

QStringList SpellCheck::userDictionaryWords(QString dict_name)
{
    DBG qDebug() << "In userDictionaryWords";
    QStringList userWords;
    // Read each word from the user dictionary.

    QFile userDictFile(userDictionaryFile(dict_name));

    if (userDictFile.open(QIODevice::ReadOnly)) {
        QTextStream userDictStream(&userDictFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        userDictStream.setCodec("UTF-8");
#endif
        QString line;

        do {
            line = userDictStream.readLine();

            if (!line.isEmpty()) {
                userWords << line;
            }
        } while (!line.isNull());

        userDictFile.close();
    }

    userWords.sort();
    return userWords;
}


void SpellCheck::dicDeltaWords(const QString &delta_path, QStringList & word_list)
{
    DBG qDebug() << "In dicDeltaWords";
    QFile deltaFile(delta_path);
    if (deltaFile.open(QIODevice::ReadOnly)) {
        QTextStream deltaStream(&deltaFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        deltaStream.setCodec("UTF-8");
#endif
        QString line;
        do {
            line = deltaStream.readLine();
            if (!line.isEmpty()) {
                word_list << line;
            }
        } while (!line.isNull());
        deltaFile.close();
    }
    return;
}


void SpellCheck::loadDictionaryNames()
{
    DBG qDebug() << "In loadDictionaryNames";
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
#ifdef Q_OS_MAC
    paths << QCoreApplication::applicationDirPath() + "/../hunspell_dictionaries";
#elif defined(Q_OS_WIN32)
    paths << QCoreApplication::applicationDirPath() + "/hunspell_dictionaries";
#elif !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    paths << Utility::LinuxHunspellDictionaryDirs();
#endif
    // Add the user dictionary directory last because anything in here
    // will override installation supplied dictionaries.
    paths << user_directory;
    foreach(QString path, paths) {
        // Find all dictionaries and add them to the avaliable list.
        QDir dictDir(path);

        if (dictDir.exists()) {
            QStringList filters;
            // Look for all .dic files.
            filters << "*.dic";
            dictDir.setNameFilters(filters);
            QStringList otherDicts = dictDir.entryList();
            foreach(QString ud, otherDicts) {
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
    DBG qDebug() << "In dictionaryDirectory";
    return Utility::DefinePrefsDir() + "/hunspell_dictionaries";
}

QString SpellCheck::userDictionaryDirectory()
{
    DBG qDebug() << "In userDictionaryDirectory";
    return Utility::DefinePrefsDir() + "/user_dictionaries";
}

QString SpellCheck::currentUserDictionaryFile()
{
    DBG qDebug() << "In currentUserDictionaryFile";
    SettingsStore settings;
    return userDictionaryDirectory() + "/" + settings.defaultUserDictionary();
}

QString SpellCheck::userDictionaryFile(QString dict_name)
{
    DBG qDebug() << "In userDictionaryFile";
    return userDictionaryDirectory() + "/" + dict_name;
}
