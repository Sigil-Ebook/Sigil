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

#include<QTextCodec>

#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
#include <QtWidgets/QInputDialog>

#include "Misc/SpellCheck.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "BookManipulation/Book.h"
#include "MainUI/MainWindow.h"
#include "Misc/Language.h"

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
# include <stdlib.h>
#endif

static const QString SETTINGS_GROUP = "spellcheck_dictionaries";

const QString NO_LINGUISTIC_CONTENT="zxx";
const QString MULTI_LANGUAGE="mul";
const QString UNDETERMINED_LANGUAGE="und";
const QString UNCODED_LANGUAGE="mis";
const QStringList NO_SPELL_LANGUAGES {NO_LINGUISTIC_CONTENT,MULTI_LANGUAGE};
const QStringList NO_DICTIONARY_LANGUAGES {NO_LINGUISTIC_CONTENT,MULTI_LANGUAGE,
            UNDETERMINED_LANGUAGE,UNCODED_LANGUAGE};

std::unique_ptr<SpellCheck> SpellCheck::m_instance = nullptr;

SpellCheck *SpellCheck::instance()
{
    if (m_instance == nullptr) {
        m_instance = std::unique_ptr<SpellCheck>(new SpellCheck());
    }

    return m_instance.get();
}

SpellCheck::SpellCheck() :
    m_hunspell(0),
    m_codec(0),
    m_wordchars(""),
    m_loadedDics {},
    m_dicAliasTable {},
    m_mainDCLanguage(QString())

{
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

    ReadSettings();
    setUpSpellCheck();

    QApplication::restoreOverrideCursor();
}

SpellCheck::~SpellCheck()
{
    if (m_hunspell) {
        delete m_hunspell;
        m_hunspell = 0;
    }
    for (auto dic : m_loadedDics.keys()){
       delete m_loadedDics.value(dic).hunspell;
       m_loadedDics.remove(dic);
    }
//    if (m_instance) {
//        delete m_instance;
//        m_instance = 0;
//    }

}

QStringList SpellCheck::userDictionaries()
{
    // Load the list of user dictionaries.
    QDir userDictDir(userDictionaryDirectory());
    QStringList user_dicts = userDictDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    user_dicts.sort();
    return user_dicts;
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

    return m_hunspell->spell(m_codec->fromUnicode(Utility::getSpellingSafeText(word)).constData()) != 0;
}

QStringList SpellCheck::suggest(const QString &word)
{
    if (!m_hunspell) {
        return QStringList();
    }

    QStringList suggestions;
    char **suggestedWords;
    int count = m_hunspell->suggest(&suggestedWords, m_codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());

    for (int i = 0; i < count; ++i) {
        suggestions << m_codec->toUnicode(suggestedWords[i]);
    }

    m_hunspell->free_list(&suggestedWords, count);
    return suggestions;
}
QStringList SpellCheck::suggest(const QString &word, const QString languageCode)
{
    HunDictionary hd{m_loadedDics.value(codeToAlias(languageCode))};

    if (!hd.hunspell) {
        return QStringList();
    }

    QStringList suggestions;

    char **suggestedWords;
    int count = hd.hunspell->suggest
            (&suggestedWords, hd.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());

    for (int i = 0; i < count; ++i) {
        suggestions << hd.codec->toUnicode(suggestedWords[i]);
    }

    hd.hunspell->free_list(&suggestedWords, count);
    return suggestions;
}
QStringList SpellCheck::suggestML(const QString &lword)
{
    QStringList list{lword.split(',')};
    QString languageCode{list.first()};
    QString word{list.last()};

    HunDictionary hd{m_loadedDics.value(codeToAlias(languageCode))};

    if (!hd.hunspell) {
        return QStringList();
    }

    QStringList suggestions;

    char **suggestedWords;
    int count = hd.hunspell->suggest
            (&suggestedWords, hd.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());

    for (int i = 0; i < count; ++i) {
        suggestions << hd.codec->toUnicode(suggestedWords[i]);
    }

    hd.hunspell->free_list(&suggestedWords, count);
    return suggestions;
}
void SpellCheck::clearIgnoredWords()
{
    m_ignoredWords.clear();
    reloadDictionary();
}

void SpellCheck::ignoreWord(const QString &word)
{
    ignoreWordInDictionary(word);

    m_ignoredWords.append(word);
}

const bool SpellCheck::ignoreWord(const QString &word, const QString &langCode)
{
    if(ignoreWordInDictionary(word,langCode)){
    m_ignoredWords.append(word);
    return true;
    }
    return false;
}
void SpellCheck::ignoreWordInDictionary(const QString &word)
{
    if (!m_hunspell) {
        return;
    }

    m_hunspell->add(m_codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());
}

const bool SpellCheck::ignoreWordInDictionary(const QString &word, const QString &langCode)
{
    //not time critical so direct struct;
    HunDictionary hd{m_loadedDics.value(codeToAlias(langCode))};

    if (!hd.hunspell) {
        return false;
    }

    hd.hunspell->add(hd.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());
    return true;
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
    // Create a new hunspell object.
    m_hunspell = new Hunspell(aff.toLocal8Bit().constData(), dic.toLocal8Bit().constData());

    // Note: these are encoded hyphenation dictionaries and their entries are not
    // meaningful words in and of themselves.
    // So the following makes no sense and is not accompishing anything.
    // That said, look into getting the raw hyphenation lists that were used
    // to generate these hyph_dic files for libhyphen

    // QString hyph_dic = QString("%1hyph_%2.dic").arg(m_dictionaries.value(name)).arg(name);
    // Load the hyphenation dictionary if it exists.
    // if (QFile::exists(hyph_dic)) {
    //    m_hunspell->add_dic(hyph_dic.toLocal8Bit().constData());
    //}

    // Get the encoding for the text in the dictionary.
    m_codec = QTextCodec::codecForName(m_hunspell->get_dic_encoding());

    if (m_codec == 0) {
        m_codec = QTextCodec::codecForName("UTF-8");
    }

    // Get the extra wordchars used for tokenization
    m_wordchars = m_codec->toUnicode(m_hunspell->get_wordchars());

    // Load in the words from the user dictionaries.
    foreach(QString word, allUserDictionaryWords()) {
        ignoreWordInDictionary(word);
    }

    // Reload the words in the "Ignored" dictionary.
    foreach(QString word, m_ignoredWords) {
        ignoreWordInDictionary(word);
    }
}


QString SpellCheck::getWordChars()
{
    return m_wordchars;
}

const QString SpellCheck::getWordChars(const QString lang){
    if(lang.isEmpty()||
            lang.isNull()||
            !m_loadedDics.keys().contains(m_dicAliasTable.value(lang)))
            return QString();
    return m_loadedDics[m_dicAliasTable.value(lang)].wordchars;
}

void SpellCheck::reloadDictionary()
{
    setDictionary(m_dictionaryName, true);
}

void SpellCheck::addToUserDictionary(const QString &word, QString dict_name)
{
    // Adding to the user dictionary also marks the word as a correct spelling.
    if (word.isEmpty()) {
        return;
    }

    SettingsStore settings;
    if (dict_name.isEmpty()) {
        dict_name = settings.defaultUserDictionary();
    }

    // Ignore the word only if the dictionary is enabled
    if (settings.enabledUserDictionaries().contains(dict_name)) {
        ignoreWordInDictionary(word);
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
            userDictStream.setCodec("UTF-8");
            userDictStream << word << "\n";
            userDictFile.close();
        }
    }
}

QStringList SpellCheck::allUserDictionaryWords()
{
    QStringList userWords;
    SettingsStore settings;
    foreach (QString dict_name, settings.enabledUserDictionaries()) {
        userWords.append(userDictionaryWords(dict_name));
    }
    return userWords;
}

QStringList SpellCheck::userDictionaryWords(QString dict_name)
{
    QStringList userWords;
    // Read each word from the user dictionary.

    QFile userDictFile(userDictionaryFile(dict_name));

    if (userDictFile.open(QIODevice::ReadOnly)) {
        QTextStream userDictStream(&userDictFile);
        userDictStream.setCodec("UTF-8");
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
    return Utility::DefinePrefsDir() + "/hunspell_dictionaries";
}

QString SpellCheck::userDictionaryDirectory()
{
    return Utility::DefinePrefsDir() + "/user_dictionaries";
}

QString SpellCheck::currentUserDictionaryFile()
{
    SettingsStore settings;
    return userDictionaryDirectory() + "/" + settings.defaultUserDictionary();
}

QString SpellCheck::userDictionaryFile(QString dict_name)
{
    return userDictionaryDirectory() + "/" + dict_name;
}

//***varlog's multilanguage

bool SpellCheck::spell(const QString word, const QString languageCode)
{
    QString dic(codeToAlias(languageCode));

    if (m_loadedDics.contains(dic)) {
            return m_loadedDics[dic].hunspell->
                    spell(m_loadedDics[dic].codec->
                          fromUnicode(Utility::getSpellingSafeText(word)).constData()) != 0;
    }
    return true;
}

const QString SpellCheck::findDictionary(const QString languageCode)

{
    if(languageCode.isEmpty()) return(QString());
    QStringList dics{dictionaries()};
    QString dicName{codeToAlias(languageCode)};
    QStringList ldics;

    if(dics.contains(dicName)) return(dicName);
    foreach(QString d,dics){
        if(languageCode==getCode(d) && dics.contains(d)){
            setDictionaryAlias(languageCode,d);
            return d;
        }
        if(d.contains(dicName)) ldics.append(d);
    }
    if(ldics.length()==0){
        QString msg=QObject::tr("No suitable dictionary for \"")+languageCode+QObject::tr("\" found.");
        Utility::DisplayStdErrorDialog(msg);
        return("");
    }
    QString c;
    if(ldics.length()==1){
        c=ldics.first();
    }else{
        c= QInputDialog::getItem(nullptr,
                                     QObject::tr("Multiple Dictionaries found!"),
                                     QObject::tr("Chose dictionary for book language:"),ldics,0,false);
    }
    //chosing dictionary "ln_WHATEVER" sets this dictionary as alias for "ln"
    setDictionaryAlias(dicName,c);
    return (c);
}

void SpellCheck::loadDictionary(const QString dicName)
{
    // See if we are already have a hunspell object for this language.
    if (m_loadedDics.contains(dicName)) {
        return;
    }

    // If we don't have a dictionary we cannot continue.
    if (dicName.isEmpty() || !m_dictionaries.contains(dicName)) {
        return;
    }

    // Dictionary files to use.
    QString aff = QString("%1%2.aff").arg(m_dictionaries.value(dicName)).arg(dicName);
    QString dic = QString("%1%2.dic").arg(m_dictionaries.value(dicName)).arg(dicName);

    struct HunDictionary hd;
    // Create a new hunspell object.
    hd.hunspell = new Hunspell(aff.toLocal8Bit().constData(), dic.toLocal8Bit().constData());

    // Get the encoding for the text in the dictionary.

    hd.codec =QTextCodec::codecForName(hd.hunspell->get_dic_encoding());
    if (hd.codec == 0) {
       hd.codec = QTextCodec::codecForName("UTF-8");
    }

    // Get the extra wordchars used for tokenization
    hd.wordchars = hd.codec->toUnicode(hd.hunspell->get_wordchars());

    m_loadedDics.insert(dicName,hd);

    // Load in the words from the user dictionaries.
    foreach(QString word, allUserDictionaryWords()) {
        ignoreWordInDictionary(word);
    }

    // Reload the words in the "Ignored" dictionary.
    foreach(QString word, m_ignoredWords) {
        ignoreWordInDictionary(word);
    }
}
void SpellCheck::loadDictionaryForLang(const QString languageCode)
{
    //languageCode is ln or ln-LN
    QString dicName{codeToAlias(languageCode)};
    loadDictionary(dicName);
    //if(!m_dicAliasTable.contains(languageCode)) m_dicAliasTable.insert(getCode(dicName),dicName);
    if(!m_dicAliasTable.contains(languageCode)){
        m_dicAliasTable.insert(languageCode,dicName);
    }
}

void SpellCheck::unloadDictionary(const QString name){

    if(m_loadedDics.contains(name)){
        delete m_loadedDics.value(name).hunspell;
        m_loadedDics.remove(name);
    }

}

const QStringList SpellCheck::alreadyLoadedDics(){
    return m_loadedDics.keys(); //implicit convertion?
}

void SpellCheck::setDCLanguages(const QList<QVariant> dclangs){
    //we have new book
    m_DCLanguages.clear();
    for(auto l:dclangs){
        QString lang{l.toString()};
        if(!NO_DICTIONARY_LANGUAGES.contains(lang)){
            m_DCLanguages<<lang;
        }
    }
    m_mainDCLanguage=m_DCLanguages.first();
    setUpNewBook();
}

void SpellCheck::setUpNewBook(){
    SettingsStore settings;
    if(settings.setUnloadCurrentDIctionaries())
        //unload previously used dictionaries
        for(auto dic:m_loadedDics.keys()){
            QStringList list{aliasToCode(dic)};
            if(!list.isEmpty()){
                if(!list.contains(m_mainDCLanguage)) unloadDictionary(dic);
            }
        }

    if(settings.setLoadMainLanguageDictionary()){
        if(!m_mainDCLanguage.isEmpty()){
            loadDictionaryForLang(m_mainDCLanguage);
        }
    }
    if(settings.setLoadAllLanguagesDIctionaries()){
        for(auto code:m_DCLanguages){
            loadDictionaryForLang(code);
        }
    }
}

QString SpellCheck::getMainDCLanguage(){
    return m_mainDCLanguage;
}

void SpellCheck::setDictionaryAlias(const QString lang, const QString dic){
    removeDictionaryAlias(lang);
    m_dicAliasTable.insert(lang,dic);
}

void SpellCheck::removeDictionaryAlias(const QString lang){
    if(m_dicAliasTable.contains(lang)){
        QString d{codeToAlias(lang)};
        m_dicAliasTable.remove(lang);
        //check if some other language uses it
        if(aliasToCode(d).length()==0){
            unloadDictionary(d);
        }
    }
}

const QString SpellCheck::codeToAlias(const QString languageCode){
    QString alias{m_dicAliasTable.value(languageCode,QString())};
    QString dic{languageCode};
    dic=(alias.isEmpty())?dic.replace("-","_"):alias;
    return dic;
}

const QStringList SpellCheck::aliasToCode(const QString dic){
     return m_dicAliasTable.keys(dic);
 }

const QString SpellCheck::getCode(const QString dicName){
    QString code{dicName};
    code=code.replace('_','-');
    QStringList list{code.split('-')};
    QString suffix{list.last()};
    suffix=suffix.toLower();
    if(suffix==list.first()) return suffix;
    return code;
}

const bool SpellCheck::isLoaded(const QString code){
    QString alias{codeToAlias(code)};
    QStringList dics{alreadyLoadedDics()};
    if(dics.contains(alias)) return true;
    return false;
}

void SpellCheck::WriteSettings(){
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.remove("");
    //aliases
    QStringList codes{m_dicAliasTable.keys()};
    settings.beginWriteArray("aliases",codes.size());
    for(int i=0;i<codes.size();i++){
        settings.setArrayIndex(i);
        settings.setValue("code",codes.at(i));
        settings.setValue("alias",m_dicAliasTable.value(codes.at(i)));
    }
    settings.endArray();
    //sessions dictionaries
    QStringList dics{m_loadedDics.keys()};
    settings.beginWriteArray("session_dictionary",dics.size());
    for(int i=0;i<dics.size();i++){
        settings.setArrayIndex(i);
        settings.setValue("code",dics.at(i));
    }
    settings.endArray();
    settings.endGroup();
}
void SpellCheck::ReadSettings(){
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    int size{settings.beginReadArray("aliases")};
    if(size>0){
        m_dicAliasTable.clear();
        for(int i=0;i<size;i++){
            settings.setArrayIndex(i);
            m_dicAliasTable.insert(
                        settings.value("code").toString(),
                        settings.value("alias").toString());
        }
        settings.endArray();
    }else{
        m_dicAliasTable={{"en","en_GB"},{"en-UK","en_GB"}};
    }
      size=settings.beginReadArray("session_dictionary");
      for(int i=0;i<size;i++){
          settings.setArrayIndex(i);
          m_lastSessionDics.append(settings.value("code").toString());
      }
    settings.endArray();
    settings.endGroup();
}

void SpellCheck::setUpSpellCheck(){
    SettingsStore settings;
    if(settings.setLoadLastSessionDictionaries()){
        for(int i=0;i<m_lastSessionDics.size();i++){
            loadDictionary(m_lastSessionDics.at(i));
        }
    }
}
