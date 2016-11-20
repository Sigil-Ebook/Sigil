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
    m_ignoredMLWords {},
    m_loadedDics {},
    m_dicAliasTable {},
    m_DCLanguages {},
    m_mainDCLanguage(QString()),
    m_lastSessionDicts {}

{
    // There is a considerable lag involved in loading the Spellcheck dictionaries
    QApplication::setOverrideCursor(Qt::WaitCursor);
    loadDictionaryNames();
    // Create the user dictionary word list directiory if necessary.
    createUserDictionary(currentUserDictionaryFile());

    ReadSettings();
    setUpSpellCheck();

    QApplication::restoreOverrideCursor();
}

SpellCheck::~SpellCheck()
{
    for (auto dic : m_loadedDics.keys()){
       delete m_loadedDics.value(dic).hunspell;
       m_loadedDics.remove(dic);
    }
}

QStringList SpellCheck::userDictionaries()
{
    // Load the list of user dictionaries.
    QDir userDictDir {userDictionaryDirectory()};
    QStringList user_dicts {userDictDir.entryList(QDir::Files | QDir::NoDotAndDotDot)};
    //remove language sub-dictionaries
    for(auto dict:user_dicts){
        if(dict.contains(QChar('.'))){
            user_dicts.removeOne(dict);
        }
    }
    user_dicts.sort();
    return user_dicts;
}

const QStringList SpellCheck::userDictLaunguages(const QString userDictName){
    QString path {userDictionaryDirectory()};
    QDir userDictDir {path};

    QFile mainDict {path + "/" + userDictName};
    if(mainDict.size()>0) return QStringList()<<"mul";
    QStringList filters {{userDictName+"[.]*"}};
    QStringList dicts {userDictDir.entryList(filters)};
    QStringList langs {};
    for(auto d:dicts){
        QFile f {path+"/"+d};
        //if(f.size()>0){
            langs<<d.split(QChar('.')).last();
        //}
    }
    return langs;
}

QStringList SpellCheck::dictionaries()
{
    loadDictionaryNames();
    QStringList dicts;
    dicts = m_dictionaries.keys();
    dicts.sort();
    return dicts;
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


void SpellCheck::clearMLIgnoredWords(const QString dicName)
{
    m_ignoredMLWords.remove(dicName);
    reloadMLDictionary(dicName);
}

void SpellCheck::clearAllMLIgnoredWords(){
    if(m_ignoredMLWords.isEmpty()) return;
    QStringList dicts {m_ignoredMLWords.keys()};
    m_ignoredMLWords.clear();
    for(auto dict:dicts){
        unloadDictionary(dict);
        loadDictionary(dict);
    }
}

const bool SpellCheck::ignoreMLWord(const QString lword){
    QStringList lw{lword.split(QChar(','))};
    return _ignoreMLWord(lw.last(),lw.first());
}

const bool SpellCheck::_ignoreMLWord(const QString word, const QString langCode)
{
    QString dicName{codeToAlias(langCode)};
    if(_ignoreMLWordInDictionary(word,dicName)){
        m_ignoredMLWords[dicName].append(word);
        return true;
    }
    return false;
}

const bool SpellCheck::ignoreMLWordInDictionary(const QString lword){
        QStringList lw{lword.split(QChar(','))};
        QString dicName{codeToAlias(lw.first())};
        return _ignoreMLWordInDictionary(lw.last(),dicName);
}

const bool SpellCheck::_ignoreMLWordInDictionary(const QString word, const QString dictName)
{
    if(!m_loadedDics.contains(dictName)){
        return false;
    }
    HunDictionary hd{m_loadedDics[dictName]};

    if (!hd.hunspell) {
        return false;
    }

    hd.hunspell->add(hd.codec->fromUnicode(Utility::getSpellingSafeText(word)).constData());
    return true;
}

const QString SpellCheck::getMLWordChars(const QString lang){
    if(lang.isEmpty()||
            lang.isNull()||
            !m_loadedDics.keys().contains(m_dicAliasTable.value(lang)))
            return QString();
    return m_loadedDics[m_dicAliasTable.value(lang)].wordchars;
}

void SpellCheck::reloadMLDictionary(const QString lang)
{
    QString dic{codeToAlias(lang)};
    unloadDictionary(dic);
    loadDictionary(dic);
}
void SpellCheck::addToUserDictionary(const QString lword, const QString dictName){
    QStringList l{lword.split(QChar(','))};
    QString suffix{codeToAlias(l.first())};

    _addToUserDictionary(l.last(),suffix,dictName);
}

void SpellCheck::_addToUserDictionary(const QString word, const QString dictCode, QString dictName)
{
    // Adding to the user dictionary also marks the word as a correct spelling.
    if (word.isEmpty()) {
        return;
    }

    SettingsStore settings;
    if (dictName.isEmpty()) {
        dictName = settings.defaultUserDictionary();
    }

    QString dict {(dictCode.isEmpty())?dictName:dictName+QChar('.')+dictCode};

    if(!createUserDictionary(dict)){
        Utility::DisplayStdErrorDialog(QObject::tr("Could not create dictionary file!"),dict);
        return;
    }

    // Ignore the word only if the dictionary is enabled
    if (settings.enabledUserDictionaries().contains(dictName)) {
        _ignoreMLWordInDictionary(word,dictCode);
    }
    QString dictPath {userDictionaryDirectory()+"/"+dict};
    if (!userDictionaryWords(dictPath).contains(word)) {

        QFile userDictFile(dictPath);


        // Try to open the file to add the word.
        if (userDictFile.open(QIODevice::Append)) {
            QTextStream userDictStream(&userDictFile);
            userDictStream.setCodec("UTF-8");
            userDictStream << word << "\n";
            userDictFile.close();
        }
    }
}

QStringList SpellCheck::allUserMLDictionaryWords(const QString alias){
    QStringList userWords {};
    SettingsStore settings;
    foreach (QString dict_name, settings.enabledUserDictionaries()) {
        userWords.append(userDictionaryWords(dict_name));
        userWords.append(userDictionaryWords(dict_name+QChar('.')+alias));
    }
    userWords.sort();
    return userWords;
}

QStringList SpellCheck::userDictionaryWords(QString dict_name)
{
    QStringList userWords {};
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

const QString SpellCheck::currentUserDictionaryFile()
{
    SettingsStore settings;
    return settings.defaultUserDictionary();
}

const QString SpellCheck::userDictionaryFile(const QString dict_name)
{
    return userDictionaryDirectory() + "/" + dict_name;
}


bool SpellCheck::spellML(const QString word, const QString languageCode)
{
    QString dic(codeToAlias(languageCode));
    if(NO_DICTIONARY_LANGUAGES.contains(languageCode)) return true;
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

void SpellCheck::loadDictionary(const QString dictName)
{
    // See if we are already have a hunspell object for this language.
    if (m_loadedDics.contains(dictName)) {
        return;
    }

    // If we don't have a dictionary we cannot continue.
    if (dictName.isEmpty() || !m_dictionaries.contains(dictName)) {
        return;
    }


    qDebug()<<"loading dictionary "<<dictName;
    // Dictionary files to use.
    QString aff = QString("%1%2.aff").arg(m_dictionaries.value(dictName)).arg(dictName);
    QString dic = QString("%1%2.dic").arg(m_dictionaries.value(dictName)).arg(dictName);

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

    m_loadedDics.insert(dictName,hd);

    // Load in the words from the user dictionaries.
    foreach(QString word, allUserMLDictionaryWords(dictName)) {
        _ignoreMLWordInDictionary(word,dictName);
    }

    // Reload the words in the "Ignored" dictionary.
    if(m_ignoredMLWords.contains(dictName)){
    for(auto word:m_ignoredMLWords[dictName]) {
        _ignoreMLWordInDictionary(word,dictName);
    }
    }
}
void SpellCheck::loadDictionaryForLang(const QString languageCode)
{
    //languageCode is ln or ln-LN
    QString dicName{codeToAlias(languageCode)};
    loadDictionary(dicName);
    if(!m_dicAliasTable.contains(languageCode)){
        m_dicAliasTable.insert(languageCode,dicName);
    }
}

void SpellCheck::unloadDictionary(const QString dictName){

    if(m_loadedDics.contains(dictName)){
        qDebug()<<"unloading dictionary "<<dictName;
        delete m_loadedDics[dictName].hunspell;
        m_loadedDics.remove(dictName);
    }

}

const QStringList SpellCheck::alreadyLoadedDics(){
    return m_loadedDics.keys(); //implicit convertion?
}

void SpellCheck::reloadAllDictionaries(){
    QStringList loadedDicts {alreadyLoadedDics()};
    QApplication::setOverrideCursor(Qt::WaitCursor);

    for(auto dict:loadedDicts){
        unloadDictionary(dict);
        loadDictionary(dict);
    }
    QApplication::restoreOverrideCursor();
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
    if(settings.setLoadAllLanguagesDictionaries()){
        for(auto code:m_DCLanguages){
            loadDictionaryForLang(code);
        }
    }
}

const QString SpellCheck::getMainDCLanguage(){
    return m_mainDCLanguage;
}

void SpellCheck::setDictionaryAlias(const QString lang, const QString dictName){
    removeDictionaryAlias(lang);
    m_dicAliasTable.insert(lang,dictName);
}

void SpellCheck::removeDictionaryAlias(const QString lang){
    if(m_dicAliasTable.contains(lang)){
        QString dict {m_dicAliasTable[lang]};
        m_dicAliasTable.remove(lang);
        //check if some other language uses coresponding dictionary
        if(!m_dicAliasTable.values().contains(dict)){
            //no longer needed
            unloadDictionary(dict);
        }
    }
}

const QString SpellCheck::codeToAlias(const QString languageCode){
    QString alias{m_dicAliasTable.value(languageCode,QString())};
    QString dict{languageCode};
    dict=(alias.isEmpty())?dict.replace("-","_"):alias;
    if(!m_dictionaries.contains(dict)){
        if(dict.contains(QChar('_'))){
            //try for ln instead of ln_LN
            dict=dict.split(QChar('_')).first();
        }else{
            //or for ln_LN instead of ln
            dict=dict+QChar('_')+dict.toUpper();
        }
        if(!m_dictionaries.contains(dict)) return QString();
        setDictionaryAlias(languageCode,dict);
    }
    return dict;
}

const QStringList SpellCheck::aliasToCode(const QString dictName){
     return m_dicAliasTable.keys(dictName);
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
          m_lastSessionDicts.append(settings.value("code").toString());
      }
    settings.endArray();
    settings.endGroup();
}

void SpellCheck::setUpSpellCheck(){
    SettingsStore settings;
    if(settings.setLoadLastSessionDictionaries()){
        for(int i=0;i<m_lastSessionDicts.size();i++){
            loadDictionary(m_lastSessionDicts.at(i));
        }
    }
}
 bool SpellCheck::isSpellable(const QString lang){
     return !NO_DICTIONARY_LANGUAGES.contains(lang);
 }

 bool SpellCheck::createUserDictionary(const QString userDictName){

     const QString user_directory{userDictionaryDirectory()};
     QDir userDir{(user_directory)};
     QFile userFile{user_directory+"/"+userDictName};

     if(userFile.exists()) return true;

     if (!userDir.exists()) {
         userDir.mkpath(user_directory);
     }


     bool success{true};
     if (!userFile.exists()) {
         if ((success=userFile.open(QIODevice::WriteOnly))) {
             userFile.close();
         }
     }
     //if created must be set used
//     SettingsStore settings;
//     QStringList edList{settings.enabledUserDictionaries()};
//     edList<<dicName;
//     settings.setEnabledUserDictionaries(edList);
     return success;
 }
