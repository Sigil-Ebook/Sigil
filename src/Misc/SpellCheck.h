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
#ifndef SPELLCHECK_H
#define SPELLCHECK_H
#include <memory>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QSharedPointer>
#include <QtCore/QTextCodec>

class Hunspell;
class QStringList;
class QTextCodec;
class HTMLSpellCheck;

/**
 * Singleton.
 */
class SpellCheck
{
public:
    static SpellCheck *instance();
    ~SpellCheck();

    struct HunDictionary{
        Hunspell *hunspell{nullptr};
        QTextCodec *codec{nullptr};
        QString wordchars{""};
    };

    QStringList userDictionaries();
    QStringList dictionaries();
    QString currentDictionary() const;
    bool spell(const QString &word);
    QStringList suggest(const QString &word);
    QStringList suggest(const QString &word,const QString languageCode);
    QStringList suggestML(const QString &lword);
    void clearIgnoredWords();
    void ignoreWord(const QString &word);
    const bool ignoreWord(const QString &word, const QString &langCode);
    void ignoreWordInDictionary(const QString &word);
    const bool ignoreWordInDictionary(const QString &word, const QString &langCode);

    QString getWordChars();
    const QString getWordChars(const QString lang);

    void setDictionary(const QString &name, bool forceReplace = false);
    void reloadDictionary();

    void addToUserDictionary(const QString &word, QString dict_name = "");
    QStringList allUserDictionaryWords();
    QStringList userDictionaryWords(QString dict_name);


    /**
     * The location of the user dictionary directories.
     */
    static QString dictionaryDirectory();
    static QString userDictionaryDirectory();
    static QString currentUserDictionaryFile();
    static QString userDictionaryFile(QString dict_name);

    void loadDictionaryNames();

    /**
     * varlog's multilanguage;
     */
     const QString findDictionary(const QString languageCode);
     void loadDictionaryForLang(const QString languageCode);
     void unloadDictionary(const QString name);
     const QStringList alreadyLoadedDics();
     void setDCLanguages(const QList<QVariant> dclangs);
     QString getMainDCLanguage();
     const QString codeToAlias(const QString languageCode);
     const QStringList aliasToCode(const QString dic);
     const bool isLoaded(const QString code);
     void setDictionaryAlias(const QString lang, const QString dic);
     void removeDictionaryAlias(const QString lang);
     void WriteSettings();
     bool spell(const QString word, const QString languageCode);
private:
     void loadDictionary(const QString dicName);
     void ReadSettings();
     void setUpSpellCheck();
     const QString getCode(const QString dicName);
     void setUpNewBook();

     //end varlog

private:
    SpellCheck();

    Hunspell *m_hunspell;
    QTextCodec *m_codec;
    QString m_wordchars;
    QString m_dictionaryName;
    //
    QHash<QString, QString> m_dictionaries;
    QStringList m_ignoredWords;

    static std::unique_ptr<SpellCheck> m_instance;

    QMap<QString,HunDictionary> m_loadedDics;
    QMap<QString,QString> m_dicAliasTable;
    QStringList m_DCLanguages;
    QString m_mainDCLanguage;
    QStringList m_lastSessionDics;

};

#endif // SPELLCHECK_H
