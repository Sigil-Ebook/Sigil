/************************************************************************
**
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
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

#ifndef HTML_PRETTY_PRINT
#define HTML_PRETTY_PRINT

#include <QList>

class QChar;
class QString;
class QStringList;

class HTMLPrettyPrint
{
public:
    HTMLPrettyPrint(const QString &source);
    ~HTMLPrettyPrint();

    QString prettyPrint();

    QStringList inlineTags();
    void resetInlineTags();

    void setIndentLevel(int level);
    void setIndentCharacter(QChar c);
    void setIndentCharacterCount(unsigned int count);
    void setIgnoreInline(bool ignore);
    void setInlineTags(QStringList tags);

private:
    typedef enum {
        TOKEN_TYPE_TAG,
        TOKEN_TYPE_OPEN_TAG,
        TOKEN_TYPE_CLOSE_TAG,
        TOKEN_TYPE_SELF_CLOSING_TAG,
        TOKEN_TYPE_COMMENT,
        TOKEN_TYPE_DECL,
        TOKEN_TYPE_XML_DECL,
        TOKEN_TYPE_DOC_TYPE,
        TOKEN_TYPE_TEXT
    } TOKEN_TYPE;

    typedef struct {
        TOKEN_TYPE type;
        size_t start;
        size_t len;
        QString tag;
    } HTMLToken;

    void tokenize();
    QString cleanSegement(const QString &source);
    TOKEN_TYPE tokenType(const QString &source);
    QString tag(const QString &source);

    QString m_source;
    QList<HTMLToken *> m_tokens;
    int m_level;
    QChar m_indentChar;
    unsigned int m_indentCharCount;
    bool m_ignoreInline;
    QStringList m_inlineTags;
};

#endif

