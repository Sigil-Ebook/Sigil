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

#include <QChar>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "HTMLPrettyPrint.h"

static QStringList defaultInlineTags = QStringList()
    << "a"
    << "abbr"
    << "acronym"
    << "audio"
    << "b"
    << "bdo"
    << "big"
    << "button"
    << "cite"
    << "del"
    << "dfn"
    << "em"
    << "i"
    << "img"
    << "ins"
    << "input"
    << "label"
    << "map"
    << "kbd"
    << "object"
    << "q"
    << "ruby"
    << "samp"
    << "select"
    << "small"
    << "span"
    << "strong"
    << "sub"
    << "sup"
    << "textarea"
    << "tt"
    << "var"
    << "video";

HTMLPrettyPrint::HTMLPrettyPrint(const QString &source)
    : m_source(source),
      m_inlineAsBlock(false),
      m_inlineTags(defaultInlineTags)
{
    tokenize();
}

HTMLPrettyPrint::~HTMLPrettyPrint()
{
    Q_FOREACH (HTMLToken *t, m_tokens) {
        delete t;
        t = 0;
    }
}

QString HTMLPrettyPrint::prettyPrint()
{
    int level = 0;
    bool in_pre = false;
    QStringList builder;
    QString segment;
    HTMLToken *last_token = 0;

    Q_FOREACH (HTMLToken *token, m_tokens) {
        if (!token) {
            continue;
        }

        segment = m_source.mid(token->start, token->len);
        if (!in_pre) {
            segment = cleanSegement(segment);
        }
        if (segment.trimmed().isEmpty()) {
            continue;
        }

        if (last_token && (token->type == TOKEN_TYPE_TEXT && last_token->type == TOKEN_TYPE_OPEN_TAG)) {
            // Indent text if it's under an open tag.
            level++;
        } else {
            if (token->type == TOKEN_TYPE_CLOSE_TAG) {
                // Only reduce indent if we're following text, or a close tag.
                if (last_token && ((last_token->type == TOKEN_TYPE_TEXT) || (last_token->type == TOKEN_TYPE_CLOSE_TAG))) {
                    level--;
                }
            // Open tags should only indented further under certain conditions.
            } else if ((!last_token && token->type == TOKEN_TYPE_OPEN_TAG) || (last_token && last_token->type == TOKEN_TYPE_OPEN_TAG)) {
                level++;
            }

            if (token->tag == "pre" || token->tag == "code") {
                in_pre = !in_pre;
            }
        }

        if (m_inlineAsBlock || (!m_inlineTags.contains(token->tag) && (last_token && !m_inlineTags.contains(last_token->tag)))) {
            if (level > 0 && !in_pre) {
                builder.append(QString("   ").repeated(level));
            }
        }

        if (!m_inlineAsBlock && (m_inlineTags.contains(token->tag))) {
            if (builder.last() == "\n") {
                builder.removeLast();
            }
        }
        if (!m_inlineAsBlock && (token->type != TOKEN_TYPE_TEXT && last_token && m_inlineTags.contains(last_token->tag))) {
            builder.append("\n");
            if (level > 0 && !in_pre) {
                builder.append(QString("   ").repeated(level));
            }
        }

        builder.append(segment);

        if (m_inlineAsBlock || !m_inlineTags.contains(token->tag)) {
            builder.append("\n");
        }

        last_token = token;
    }

    return builder.join("");
}

QStringList HTMLPrettyPrint::inlineTags()
{
    return m_inlineTags;
}

void HTMLPrettyPrint::resetInlineTags()
{
    m_inlineTags = defaultInlineTags;
}

void HTMLPrettyPrint::setInlineAsBlock(bool asBlock)
{
    m_inlineAsBlock = asBlock;
}

void HTMLPrettyPrint::setInlineTags(QStringList tags)
{
    m_inlineTags = tags;
}

void HTMLPrettyPrint::tokenize()
{
    QChar c;
    size_t start = 0;

    for (int i = 0; i < m_source.size(); ++i) {
        c = m_source.at(i);

        if (c == '<' || c == '>') {
            HTMLToken *token = new HTMLToken();
            token->start = start;
            token->len = i - start;

            start = i;
            if (c == '<') {
                token->type = TOKEN_TYPE_TEXT;
            } else if (c == '>') {
                token->len++;

                QString segment = m_source.mid(token->start, token->len);
                token->type = tokenType(segment);
                token->tag = tag(segment);

                i++;
                start++;
            }

            m_tokens.append(token);
        }
    }

    // Trailing text.
    if (start != m_source.size()) {
        HTMLToken *token = new HTMLToken();
        token->start = start;
        token->len = m_source.size() - start;
        token->type = TOKEN_TYPE_TEXT;
        m_tokens.append(token);
    }
}

QString HTMLPrettyPrint::cleanSegement(const QString &source)
{
    QString segment = source;

    segment = segment.replace(QRegExp("(\\r\\n)|\\n|\\r"), " ");
    segment = segment.replace(QRegExp("\\s{2,}"), " ");

    return segment;
}

HTMLPrettyPrint::TOKEN_TYPE HTMLPrettyPrint::tokenType(const QString &source)
{
    if (source.startsWith("</")) {
        return TOKEN_TYPE_CLOSE_TAG;
    } else if (source.endsWith("/>")) {
        return TOKEN_TYPE_SELF_CLOSING_TAG;
    } else if (source.startsWith("<!--")) {
        return TOKEN_TYPE_COMMENT;
    } else if (source.startsWith("<?")) {
        return TOKEN_TYPE_XML_DECL;
    } else if (source.startsWith("<!")) {
        return TOKEN_TYPE_DOC_TYPE;
    }
    return TOKEN_TYPE_OPEN_TAG;
}

QString HTMLPrettyPrint::tag(const QString &source)
{
    QString tag;
    QRegExp tag_cap("</?\\s*([^\\s/>]+)");

    if (tag_cap.indexIn(source) > -1) {
        tag = tag_cap.cap(1);
    }

    return tag.toLower();
}

