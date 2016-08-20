/*
* hasTag() is based on parsetag() from quickparser.py:
* Copyright (c) 2014 Kevin B. Hendricks, John Schember,
* and Doug Massay

* Copyright (c)2016 varlog (http://www.mobileread.com/forums/member.php?u=182437).
* All rights reserved.

* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.

* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE FREEBSD PROJECT ``AS IS'' AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
* NO EVENT SHALL THE FREEBSD PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


//DEBUG
#include <QtWidgets/QPlainTextEdit>
//DEBUG

#include <QtCore/QPair>

#include "QuickSerialHtmlParser.h"
#include "Misc/Utility.h"
#include "Misc/SpellCheck.h"
#include "Misc/HTMLSpellCheck.h"

const QString XMLHEADER{"?xml"};
const QString HTMLCOMMENT{"!--"};
const QString DOCTYPE{"!DOCTYPE"};
const QString CDATA{"![CDATA["};
const QString SCRIPT{"script"};
const QString SPECIAL{"special"};
const QString STYLE{"style"};

//only QPair seems to be able to get initialised liks this
const QMap<QString,QPair<TagType,QString> > SPECIAL_TAGS{
    { XMLHEADER,{TagType::XMLHEADER,"?>"}},
    { HTMLCOMMENT,{TagType::COMMENT,"-->"}},
    { DOCTYPE,{TagType::DOCTYPE,">"}},
    { CDATA,{TagType::CDATA,"]]>"}},
    { STYLE,{TagType::STYLE,"</style>"}}

};

const QStringList SPECIAL_ATTRIBUTES{
    HTMLCOMMENT,DOCTYPE,CDATA,STYLE
};

const QString TAG_CHARACTERS{QChar('>')+QChar('/')};
const QString WHITESPACE{QChar(' ')+QChar('\t')+QChar('\n')+QChar('\r')+QChar('\f')};
const QString STOP_CHARACTERS{TAG_CHARACTERS+WHITESPACE};
const QString QUOTES{QChar('\"')+QChar('\'')};
const QString ALL_STOPS{TAG_CHARACTERS+WHITESPACE+QUOTES};


QuickSerialHtmlParser::QuickSerialHtmlParser(const QString parentLanguage,
                                             const bool haveDOM,
                                             const bool haveLMap, const bool isHighlighter):
  m_DOME{},
  m_tagSTACK{},
  m_haveDome{haveDOM},
  m_haveLMap{haveLMap},
  m_documentParsed{false},
  m_Highlighter{isHighlighter},
  m_charsParsed{0}
{
    QString lang{parentLanguage};
    if(parentLanguage.isNull()){
        SpellCheck *sc=SpellCheck::instance();
        lang=sc->getMainDCLanguage();
    }
    setParentTag("lang",lang);
    m_tagSTACK.append(m_parentTag);
    if(m_haveDome) m_DOME.append(m_parentTag);
    if(m_haveLMap) {
        m_languageMAP.append({0,0,lang});
    }
    clean();
}

QuickSerialHtmlParser::~QuickSerialHtmlParser(){
}

void QuickSerialHtmlParser::setParentTag(const QString attribute, const QString value){
    m_parentTag.name ="parentTag";
    m_parentTag.type =TagType::PARENT;
    m_parentTag.length=-1;
    m_parentTag.offset=-1;
    m_parentTag.attribute.insert(attribute,value);
   // reset();
}

void QuickSerialHtmlParser::reset(){
    clean();
    m_documentParsed=false;
    m_tagSTACK = {};
    m_DOME = {};
    m_languageMAP = {};
    if(m_parentTag.type==TagType::PARENT){
        if(m_haveDome) m_DOME.append(m_parentTag);
            m_tagSTACK.append(m_parentTag);
        if(m_haveLMap)
            m_languageMAP.append({0,0,m_parentTag.attribute.value("lang")});
    }
    //m_charsParsed is used after parsing is completed
    //to get right position in parsed text,
    //so it is never cleaned
}

void QuickSerialHtmlParser::clean(){
    m_currentTag.name.clear();
    m_currentTag.type=TagType::NONE;
    m_currentTag.length=0;
    m_currentTag.offset=0;
    m_currentTag.attribute.clear();
    m_tagComplete =true;
    m_tagStart=0;
    m_whatHappendSoFar=QString();
    m_seek= "";
    m_startPos=0 ;
}

const bool QuickSerialHtmlParser::hasTag(const QString &orig_text, const int start){

int len{orig_text.length()};
int pos{start};
QString text{orig_text};
int nb{0};
int end{0};

//check if we are in the middle of something
if(!m_tagComplete){
    // we have a new chunk, start is always 0 here but
    // it starts with phony QChar ' '
    text=orig_text.mid(1,len-start);

    //	we should already have the tag name:
    //  highlighter is providing lines of
    // text and the name cannot be divided by line break
    // nevertheless to be truly serial
    if(m_currentTag.name.isEmpty()){
        //set the start as in the first chunk
        pos=m_startPos;
        text=m_whatHappendSoFar+text;
        len=text.length();
        goto GETNAME;
    }
    pos+=m_currentTag.name.length();
    if(m_currentTag.length==0){
        //check if this chunk contains end
        //m_seek is set, because name is known
        end=(m_seek==">")?
            orig_text.indexOf(QChar('>',start)):orig_text.indexOf(m_seek,start);
        if(end==-1){
            //no end in this chunk, next one please
            //set history: remove false QChar(' ') at the end
            m_whatHappendSoFar+=text.left(text.length()-1);
            m_charsParsed=orig_text.length()-start;
            return true;
        }else{
            m_charsParsed=end+m_seek.length()-1;
            text=m_whatHappendSoFar+text;
            //correct end,
            end+=m_whatHappendSoFar.length()-1;
            pos=m_tagStart+m_currentTag.name.length();
            m_tagComplete=true;
        }
    }
    goto GETATTRIBUTE;
}
//this is new text with tag, remember start position
m_startPos=start;
m_charsParsed=0;

//******
GETNAME:
while(WHITESPACE.contains(text.at(pos)) && pos<len) ++pos;
if(text.at(pos)!=QChar('<')){
    //no tag
    return false;
}
m_tagStart=pos;
//correct phony QChar(' '):     -1
//alas: offset will be tottaly wrong
//for highlighter instance of QSHP
m_currentTag.offset=pos-1;
++pos;
if(text.at(pos)==QChar('/')){
    m_currentTag.type=TagType::END;
    ++pos;
}
nb=pos;
if(text.mid(pos,4)==XMLHEADER){
    m_currentTag.offset=0;
    m_documentParsed=false;
 }

//we refuse to parse if already parsed
if(m_documentParsed) return false;

//special case: comment
if(text.mid(pos,3)==HTMLCOMMENT){
    m_currentTag.name=HTMLCOMMENT;
    m_currentTag.type=TagType::COMMENT;
    pos+=3;
    m_seek="-->";
goto GETEND;
}

while(!STOP_CHARACTERS.contains(text.at(pos))&& pos<len) ++pos;
//check if we really have whitespace
if(!STOP_CHARACTERS.contains(text.at(pos))){
    //chunk aborted in middle of the name
        m_tagComplete=false;
        m_whatHappendSoFar=text.left(text.length()-1);
    // next chunk, please
    return true;
}

//we have tag name
m_currentTag.name=text.mid(nb,pos-nb).toLower();
if(m_currentTag.name=="!doctype") m_currentTag.name=DOCTYPE;
if(m_currentTag.name=="![cdata[") m_currentTag.name=CDATA;

//DEBUG
//_debug();

// check if the tag is special one
// and set end of tag string
if(SPECIAL_TAGS.keys().contains(m_currentTag.name)){
    m_currentTag.type=SPECIAL_TAGS.value(m_currentTag.name).first;
    m_seek=SPECIAL_TAGS.value(m_currentTag.name).second;
}else{
    m_seek=">";
}

//*****
GETEND:
//check if tag ends in this chunk
//we want to have a whole tag before dealing with attributes
end=(m_seek==">")?
    text.indexOf(QChar('>'),pos):text.indexOf(m_seek,pos);
if(end==-1){
    //no end in this chunk, next one please
        m_tagComplete=false;
        m_charsParsed=orig_text.length()-start;
        m_whatHappendSoFar=text.left(text.length()-1);
    return true;
}
m_tagComplete=true;
m_charsParsed=end+m_seek.length()-start-1;

//***********
GETATTRIBUTE:

m_currentTag.length=end-m_tagStart;
len=end+1;
QString aname{""};
QString aval{""};

//special cases
if(SPECIAL_ATTRIBUTES.contains(m_currentTag.name)){
//we want the whole content in attribute special
    aname=SPECIAL;
    aval=text.mid(pos+1,end-pos);
    m_currentTag.attribute.insert(aname,aval);
    goto FINISH;
}

// others
while(pos<len &&
    !(text.indexOf(QChar('='),pos)==-1
    ||text.indexOf(QChar('='),pos)>len) ){

        while(WHITESPACE.contains(text.at(pos)) && pos<len) ++pos;
        //attreibute name
        nb=pos;
        while(pos<len && text.at(pos)!=QChar('=')) ++pos;
        // attribute names can be mixed case and are in SVG
        aname=text.mid(nb,pos-nb).trimmed();
        ++pos;
        while(WHITESPACE.contains(text.at(pos)) && pos<len) ++pos;
        //****atributte value
        if(QUOTES.contains(text.at(pos))){
            //attribute value in quotes
            QChar qt{text.at(pos)};
            ++pos;
            nb=pos;
            while(pos<len && text.at(pos)!=qt) pos++;
            aval=text.mid(nb,pos-nb);
            ++pos;
        }
        else{
            //attribute value not quoted: html5 allows it for one
            nb=pos;
            while(pos<len && !STOP_CHARACTERS.contains(text.at(pos)))
            ++pos;
            aval=text.mid(nb,pos-nb);
        }
    m_currentTag.attribute.insert(aname,aval);
}
// begin and single tag types
if(m_currentTag.type==TagType::NONE){
    m_currentTag.type=TagType::BEGIN;
    int j=text.indexOf(QChar('/'),pos);
    if(j>=0 && j<len) m_currentTag.type=TagType::SINGLE;
}

//******
FINISH:

tagParsed();
return false;
}

void QuickSerialHtmlParser::tagParsed(){

    //DEBUG
    if(!m_tagComplete){        
        Utility::DisplayStdErrorDialog(QObject::tr("I should not be here but I am!"));
        return;
    }

    if(m_currentTag.type==TagType::XMLHEADER){
        //we have a new document
        //by html5 the DOCTYPE is the start tag?
        HtmlTag tag{m_currentTag};
        reset();
        m_currentTag=tag;
    }

    if(m_haveDome){
        m_DOME.append(m_currentTag);
    }

    if(m_currentTag.type==TagType::BEGIN){
        if(m_haveLMap){
            addLangZoneBegin();
        }
        m_tagSTACK.append(m_currentTag);
        clean();
        return;
    }

    if(m_currentTag.type==TagType::END){
        if(m_currentTag.name=="html"){
            //we have end of this document
            m_documentParsed=true;
            if(m_haveLMap){
                m_languageMAP.last().end=m_currentTag.offset-1;
            }
            m_tagSTACK.removeLast();
            clean();
            return;
        }       
        if(m_tagSTACK.last().name==m_currentTag.name){
            if(m_haveLMap){
                addLangZoneEnd();
            }
            m_tagSTACK.removeLast();
            clean();
            return;
        }

        /*
         * Just do nothing
         Utility::DisplayStdErrorDialog(QObject::tr("Tag begin-end mismatch!"),
                                        QObject::tr("begin is:")+m_tagSTACK.last().name+
                                        QObject::tr(" end is:")+m_currentTag.name);

        */
    }
    clean();

}

const bool QuickSerialHtmlParser::hasAttribute(const QString attr, const HtmlTag &tag){
    if(tag.attribute.keys().contains(attr)) return true;
    return false;
     }

const QString QuickSerialHtmlParser::hasLanguage(const HtmlTag tag){
    if(hasAttribute("lang",tag)) return "lang";
    if(hasAttribute("xml:lang",tag)) return "xml:lang";
    return QString();
}

const QString QuickSerialHtmlParser::getTagAttributeValue(const QString attr,const HtmlTag &tag){
     if(hasAttribute(attr,tag)) return tag.attribute.value(attr);
    //this string is NULL, not empty!
    return QString();
}

const QString QuickSerialHtmlParser::getCurrLanguage(){
    int i{m_tagSTACK.length()-1};
    QString attr {hasLanguage(m_tagSTACK.last())};
    if(attr.isEmpty()){
        while(i>0 && attr.isEmpty()) {
            //top one is already checked
            --i;
            attr=hasLanguage(m_tagSTACK[i]);
        }
    }
    //the parent tag must be there, so we get the language
    return getTagAttributeValue(attr,m_tagSTACK[i]);
}

const int QuickSerialHtmlParser::lengthParsed(){
    return m_charsParsed;
}

const QString QuickSerialHtmlParser::getLanguage(const int cursorPos,
                                                 const QVector<languageZone> &lZone) const{
    QString lang{QString()};
    for(int i=0;i<lZone.size() && lang.isEmpty();++i){
        if(cursorPos>=lZone.at(i).begin && cursorPos<=lZone.at(i).end){
            lang=lZone.at(i).lang;
        }
    }
    return lang;
}

const QString QuickSerialHtmlParser::getLanguage(const int cursorPos) const{
    if(m_documentParsed && m_haveLMap){
        return getLanguage(cursorPos,m_languageMAP);
    }
    return QString();
}

const QVector<languageZone> QuickSerialHtmlParser::getLanguageMap(){
    return m_languageMAP;
}

const bool QuickSerialHtmlParser::inTag() const{
    return !m_tagComplete;
}

void QuickSerialHtmlParser::addLangZoneBegin(){
    QString attr{hasLanguage(m_currentTag)};
    if(!attr.isEmpty()){
        QString lang=getTagAttributeValue(attr,m_currentTag);
        if(m_languageMAP.last().lang!=lang){
            m_languageMAP.last().end=m_currentTag.offset;
            m_languageMAP.append({m_currentTag.offset+m_currentTag.length+1,0,lang});
        }
     }
}

void QuickSerialHtmlParser::addLangZoneEnd(){
    QString attr{hasLanguage(m_tagSTACK.last())};
    if(!attr.isEmpty()){
        if(m_languageMAP.last().begin==m_currentTag.offset){
            m_languageMAP.removeLast();
        }else{
            m_languageMAP.last().end=m_currentTag.offset;
        }
        //seek the language before
        QString lang{getLangFromStack(-2)};
        m_languageMAP.append({m_currentTag.offset+m_currentTag.length,0,lang});
    }
}

const QString QuickSerialHtmlParser::getLangFromStack(const int startTag){
    int i{m_tagSTACK.length()+startTag};
    QString attr {hasLanguage(m_tagSTACK.at(i))};
    if(attr.isEmpty()){
        while(i>0 && attr.isEmpty()) {
            //top one is already checked
            --i;
            attr=hasLanguage(m_tagSTACK[i]);
        }
    }
    //the parent tag must be there, so we get the language
    return getTagAttributeValue(attr,m_tagSTACK[i]);
}

void QuickSerialHtmlParser::parseText(const QString &text){
    //TODO correct phony QChar(' ') issue
    for(int i=0;i<text.length();++i){
        if(text.at(i)==QChar('<')){
            hasTag(text,i);
            i+=m_charsParsed;
        }
    }
}

void QuickSerialHtmlParser::_debug(){
    Utility::DisplayStdWarningDialog(QString::number(m_currentTag.offset),m_currentTag.name);
}
