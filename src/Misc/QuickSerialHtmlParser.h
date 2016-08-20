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

#ifndef QUICKESERIALHTMLPARSER_H
#define QUICKSERIALHTMLPARSER_H
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>

enum class TagType{
    NONE,
    XMLHEADER,
    DOCTYPE,
    BEGIN,
    END,
    SINGLE,
    COMMENT,
    CDATA,
    STYLE,
    PARENT
};

struct HtmlTag{
    QString name{""};
    TagType type{TagType::NONE};
    int offset{0};
    int length{0};
    QMap <QString ,QString > attribute{};
};

struct languageZone{
    int begin;
    int end;
    QString lang;
};

class QuickSerialHtmlParser
{

public:

    QuickSerialHtmlParser(const QString parentLanguage=QString(),
                          const bool haveDOM=false,
                          const bool haveLMap=true,
                          const bool isHighlighter=false);

    ~QuickSerialHtmlParser();

    //set phony parent tag with language grabbed from dc:language
    void setParentTag(const QString attribute, const QString value);
    //for building languageMap only at the moment
    void parseText(const QString &text);
    //parse html tag
    const bool hasTag(const QString &orig_text, const int start);
    //get the length of parsed tag
    const int lengthParsed();
    //get language from stack TODO: use getLangFromStack() for it
    const QString getCurrLanguage();
    //unused
    const QString getLanguage(const int cursorPos,const QVector<languageZone> &lZone) const;
    //get language according to cursor position
    const QString getLanguage(const int cursorPos) const;
    //unused
    const QVector<languageZone> getLanguageMap();
    const bool inTag() const;

private:
    void tagParsed();
    const bool hasAttribute(const QString attr, const HtmlTag &tag);
    const QString hasLanguage(const HtmlTag tag);
    //clean some m_'s
    void clean();
    //clean class pending new text
    void reset();

    const QString getTagAttributeValue(const QString attr, const HtmlTag &tag);
    void addLangZoneBegin();
    void addLangZoneEnd();
    const QString getLangFromStack(const int startTag);
    //DEBUG
    void _debug();

    HtmlTag m_parentTag;
    HtmlTag m_currentTag;
    QVector<HtmlTag> m_DOME;
    QVector<HtmlTag> m_tagSTACK;
    QVector<languageZone> m_languageMAP;
    bool m_haveDome;
    bool m_haveLMap;
    QString m_whatHappendSoFar;
    QString m_seek;
    bool m_tagComplete;
    bool m_documentParsed;
    bool m_Highlighter;
    int m_tagStart;
    int m_startPos;
    int m_charsParsed;

};

#endif // QUICKSERIALHTMLPARSER_H
