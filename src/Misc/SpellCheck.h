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



    QStringList userDictionaries();
    QStringList dictionaries();

    QStringList suggestML(const QString &lword);

    void clearMLIgnoredWords(const QString dicName);
    void clearAllMLIgnoredWords();

    const bool ignoreMLWord(const QString lword);
    const bool ignoreMLWordInDictionary(const QString lword);

    const QString getMLWordChars(const QString lang);

    void reloadMLDictionary(const QString lang);

    void addToUserDictionary(const QString lword, const QString dictName="");

    QStringList allUserMLDictionaryWords(const QString alias);
    QStringList userDictionaryWords(QString dict_name);


    /**
     * The location of the user dictionary directories.
     */
    static QString dictionaryDirectory();
    static QString userDictionaryDirectory();


     bool createUserDictionary(const QString userDictName);
     const QString findDictionary(const QString languageCode);
     void loadDictionaryForLang(const QString languageCode);
     void loadDictionary(const QString dictName);
     void unloadDictionary(const QString dictName);
     void reloadAllDictionaries();
     const QStringList alreadyLoadedDics();

     void setDCLanguages(const QList<QVariant> dclangs);
     const QString getMainDCLanguage();

     const QString codeToAlias(const QString languageCode);
     const QStringList aliasToCode(const QString dictName);

     void setDictionaryAlias(const QString lang, const QString dictName);
     void removeDictionaryAlias(const QString lang);

     void WriteSettings();

     bool spellML(const QString word, const QString languageCode);

     const bool isLoaded(const QString code);
     bool isSpellable(const QString lang);

     const QStringList userDictLaunguages(const QString userDictName);

private:

     struct HunDictionary{
         Hunspell *hunspell{nullptr};
         QTextCodec *codec{nullptr};
         QString wordchars{""};
     };

     void loadDictionaryNames();
     const QString currentUserDictionaryFile();
     const QString userDictionaryFile(const QString dict_name);
     const bool _ignoreMLWordInDictionary(const QString word, const QString dictName);
     void _addToUserDictionary(const QString word, const QString dictCode, QString dictName = "");
     const bool _ignoreMLWord(const QString word, const QString langCode);

     void ReadSettings();
     void setUpSpellCheck();
     const QString getCode(const QString dicName);
     void setUpNewBook();


    SpellCheck();

    QHash<QString, QString> m_dictionaries;
    QMap <QString,QStringList> m_ignoredMLWords;

    static std::unique_ptr<SpellCheck> m_instance;

    QMap<QString,HunDictionary> m_loadedDics;
    QMap<QString,QString> m_dicAliasTable;
    QStringList m_DCLanguages;
    QString m_mainDCLanguage;
    QStringList m_lastSessionDicts;

};

#endif // SPELLCHECK_H
